#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "./alarm/AlarmManager.h"
#include "./communication/QtModbusClient.h"
#include "./core/AlarmRecord.h"
#include "./core/DeviceConfig.h"
#include "./core/EngineeringValue.h"
#include "./core/RegisterValue.h"
#include "./database/DatabaseManager.h"
#include "./polling/PollingWorker.h"
#include "./ui/AlarmPanel.h"
#include "./ui/ConnectionPanel.h"
#include "./ui/HistoryPanel.h"
#include "./ui/MonitorPanel.h"
#include "./ui/PacketMonitorPanel.h"
#include "./ui/RegisterPanel.h"

#include <QDateTime>
#include <QStatusBar>
#include <QString>
#include <QTabWidget>

namespace {

QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setupMainTabs();

    resize(1200, 800);
    setWindowTitle(zh(u8"Qt Modbus RTU/TCP 工业设备监控与调试系统"));
    statusBar()->showMessage(zh(u8"未连接"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupMainTabs()
{
    // 主标签页负责承载各个功能面板，所有面板统一挂到 tabs 下管理生命周期。
    tabs = new QTabWidget(this);

    ConnectionPanel *connectionPanel = new ConnectionPanel(tabs);
    RegisterPanel *registerPanel = new RegisterPanel(tabs);
    MonitorPanel *monitorPanel = new MonitorPanel(tabs);
    AlarmPanel *alarmPanel = new AlarmPanel(tabs);
    PacketMonitorPanel *packetPanel = new PacketMonitorPanel(tabs);

    // 核心业务对象由主窗口持有，随主窗口销毁自动释放。
    QtModbusClient *modbusClient = new QtModbusClient(this);
    PollingWorker *pollingWorker = new PollingWorker(this);
    AlarmManager *alarmManager = new AlarmManager(this);
    DatabaseManager *databaseManager = new DatabaseManager(this);

    HistoryPanel *historyPanel = new HistoryPanel(databaseManager, tabs);

    tabs->addTab(connectionPanel, "设备连接");
    tabs->addTab(registerPanel, "寄存器调试");
    tabs->addTab(monitorPanel, "实时监控");
    tabs->addTab(alarmPanel, "报警记录");
    tabs->addTab(historyPanel, "历史查询");
    tabs->addTab(packetPanel, "报文日志");

    // 数据库只打开和初始化一次，采集数据与报警记录共用同一个数据库连接。
    if (databaseManager->open("modbus_hmi.db") && databaseManager->initialize()) {
        connect(pollingWorker, &PollingWorker::engineeringValueReady,
            databaseManager, &DatabaseManager::saveEngineeringValue);

        connect(alarmManager, &AlarmManager::alarmRaised,
            databaseManager, &DatabaseManager::saveAlarmRecord);
    }

    connect(databaseManager, &DatabaseManager::errorOccurred,
        this, [this](const QString &message) {
            statusBar()->showMessage("数据库错误：" + message);
    });

    connect(databaseManager, &DatabaseManager::errorOccurred,
        packetPanel, [packetPanel](const QString &message) {
            packetPanel->appendText(
                QString("[%1] DB: Error %2")
                    .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"))
                    .arg(message)
            );
    });

    // 连接面板只负责发出用户意图，真正的连接和断开由 Modbus 客户端执行。
    connect(connectionPanel, &ConnectionPanel::connectRequested,
        modbusClient, &QtModbusClient::connectDevice);

    connect(connectionPanel, &ConnectionPanel::disconnectRequested,
        modbusClient, &QtModbusClient::disconnectDevice);

    connect(connectionPanel, &ConnectionPanel::connectRequested,
        this, [this](const DeviceConfig &config) {
            statusBar()->showMessage(
                "请求连接：" + config.tcp.host + ":" + QString::number(config.tcp.port)
            );
    });

    connect(connectionPanel, &ConnectionPanel::disconnectRequested,
        this, [this]() {
            statusBar()->showMessage("已断开");
    });

    connect(modbusClient, &QtModbusClient::connected,
        this, [this]() {
            statusBar()->showMessage("已连接");
    });

    connect(modbusClient, &QtModbusClient::disconnected,
        this, [this]() {
            statusBar()->showMessage("已断开");
    });

    connect(modbusClient, &QtModbusClient::errorOccurred,
        this, [this](const QString &message) {
            statusBar()->showMessage("通信错误：" + message);
    });

    // 寄存器调试面板发起读写请求，Modbus 客户端返回结果后再刷新表格。
    connect(registerPanel, &RegisterPanel::readHoldingRegistersRequested,
        modbusClient, &QtModbusClient::readHoldingRegisters);

    connect(registerPanel, &RegisterPanel::writeSingleHoldingRegisterRequested,
        modbusClient, &QtModbusClient::writeSingleHoldingRegister);

    connect(modbusClient, &QtModbusClient::holdingRegistersRead,
        registerPanel, &RegisterPanel::displayHoldingRegisters);

    // 连接成功后启动周期轮询；断开后停止轮询，避免继续发送读请求。
    connect(modbusClient, &QtModbusClient::connected,
        pollingWorker, [pollingWorker]() {
            pollingWorker->start(1000, 0, 4);
    });

    connect(modbusClient, &QtModbusClient::disconnected,
        pollingWorker, &PollingWorker::stop);

    connect(pollingWorker, &PollingWorker::readRequested,
        modbusClient, &QtModbusClient::readHoldingRegisters);

    connect(modbusClient, &QtModbusClient::holdingRegistersRead,
        pollingWorker, &PollingWorker::onRegistersRead);

    connect(modbusClient, &QtModbusClient::errorOccurred,
        pollingWorker, &PollingWorker::onError);

    // 轮询结果先更新实时监控，再交给报警管理器判断是否越限。
    connect(pollingWorker, &PollingWorker::engineeringValueReady,
        monitorPanel, &MonitorPanel::updateValue);

    connect(pollingWorker, &PollingWorker::engineeringValueReady,
        alarmManager, &AlarmManager::checkValue);

    connect(pollingWorker, &PollingWorker::engineeringValueReady,
        this, [this](const EngineeringValue &value) {
            statusBar()->showMessage(
                QString("温度=%1 ℃  电压=%2 V  电流=%3 A  转速=%4 rpm")
                    .arg(value.temperature)
                    .arg(value.voltage)
                    .arg(value.current)
                    .arg(value.speed)
            );
    });

    // 轮询连续失败或远端异常断开时，统一交给报警管理器生成离线报警。
    connect(pollingWorker, &PollingWorker::deviceOffline,
        alarmManager, &AlarmManager::onDeviceOffline);

    connect(modbusClient, &QtModbusClient::unexpectedDisconnected,
        alarmManager, &AlarmManager::onDeviceOfflineForDevice);

    connect(pollingWorker, &PollingWorker::deviceOffline,
        this, [this]() {
            statusBar()->showMessage("设备连续通信失败，判定离线");
    });

    connect(alarmManager, &AlarmManager::alarmRaised,
        alarmPanel, &AlarmPanel::appendAlarm);

    connect(alarmManager, &AlarmManager::alarmRaised,
        this, [this](const AlarmRecord &alarm) {
            statusBar()->showMessage("报警：" + alarm.message);
    });

    // 报文日志用于观察用户操作、通信结果、报警和数据库错误。
    connect(connectionPanel, &ConnectionPanel::connectRequested,
        packetPanel, [packetPanel](const DeviceConfig &config) {
            packetPanel->appendText(
                QString("[%1] TX: Connect request %2:%3 slave=%4")
                    .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"))
                    .arg(config.tcp.host)
                    .arg(config.tcp.port)
                    .arg(config.slaveId)
            );
    });

    connect(connectionPanel, &ConnectionPanel::disconnectRequested,
        packetPanel, [packetPanel]() {
            packetPanel->appendText(
                QString("[%1] TX: Disconnect request")
                    .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"))
            );
    });

    connect(registerPanel, &RegisterPanel::readHoldingRegistersRequested,
        packetPanel, [packetPanel](int startAddress, int count) {
            packetPanel->appendText(
                QString("[%1] TX: Read holding registers start=%2 count=%3")
                    .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"))
                    .arg(startAddress)
                    .arg(count)
            );
    });

    connect(registerPanel, &RegisterPanel::writeSingleHoldingRegisterRequested,
        packetPanel, [packetPanel](int address, quint16 value) {
            packetPanel->appendText(
                QString("[%1] TX: Write single holding register address=%2 value=%3")
                    .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"))
                    .arg(address)
                    .arg(value)
            );
    });

    connect(modbusClient, &QtModbusClient::holdingRegistersRead,
        packetPanel, [packetPanel](const RegisterReadResult &result) {
            packetPanel->appendText(
                QString("[%1] RX: Holding registers read ok start=%2 count=%3")
                    .arg(result.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz"))
                    .arg(result.startAddress)
                    .arg(result.values.size())
            );
    });

    connect(modbusClient, &QtModbusClient::errorOccurred,
        packetPanel, [packetPanel](const QString &message) {
            packetPanel->appendText(
                QString("[%1] RX: Error %2")
                    .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"))
                    .arg(message)
            );
    });

    connect(alarmManager, &AlarmManager::alarmRaised,
        packetPanel, [packetPanel](const AlarmRecord &alarm) {
            packetPanel->appendText(
                QString("[%1] ALARM: %2")
                    .arg(alarm.alarmTime.toString("yyyy-MM-dd HH:mm:ss.zzz"))
                    .arg(alarm.message)
            );
    });

    setCentralWidget(tabs);
}
