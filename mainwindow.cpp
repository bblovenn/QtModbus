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
#include "./ui/AlarmHistoryPanel.h"
#include "./ui/AlarmConfigPanel.h"
#include "./ui/PacketHistoryPanel.h"
#include "./ui/DatabaseMaintenancePanel.h"
#include "./ui/PollingConfigPanel.h"
#include "./ui/TrendPanel.h"

#include <QDateTime>
#include <QStatusBar>
#include <QString>
#include <QTabWidget>
#include <QSettings> //持久化配置存储类，用于保存和加载应用程序的设置

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
    TrendPanel *trendPanel = new TrendPanel(tabs);

    // 核心业务对象由主窗口持有，随主窗口销毁自动释放。
    QtModbusClient *modbusClient = new QtModbusClient(this);
    PollingWorker *pollingWorker = new PollingWorker(this);
    AlarmManager *alarmManager = new AlarmManager(this);
    DatabaseManager *databaseManager = new DatabaseManager(this);

    HistoryPanel *historyPanel = new HistoryPanel(databaseManager, tabs);
    AlarmHistoryPanel *alarmHistoryPanel = new AlarmHistoryPanel(databaseManager, tabs);

    PacketHistoryPanel *packetHistoryPanel = new PacketHistoryPanel(databaseManager, tabs);

    DatabaseMaintenancePanel *databaseMaintenancePanel = new DatabaseMaintenancePanel(databaseManager, tabs);

    //设计模式：这是一个观察者/发布-订阅模式的变体
    //lambda 表达式（匿名函数），用于持久化记录通信报文日志
    auto appendPersistentLog =
    //捕获列表：捕获外部的两个变量，供 lambda 内部使用
    [packetPanel, databaseManager](
        const QString &category,
        const QString &direction,
        const QString &content
    ) {
        const QDateTime now = QDateTime::currentDateTime();

        packetPanel->appendText(
            QString("[%1] %2: %3")
                .arg(now.toString("yyyy-MM-dd HH:mm:ss.zzz"))
                .arg(direction)
                .arg(content)
        );

        databaseManager->savePacketLog(
            now,
            category,
            direction,
            content
        );
    };

    QSettings settings("QtModbusHmi", "ModbusIndustrialHmi");

    DeviceConfig savedDeviceConfig;   
    savedDeviceConfig.mode = static_cast<ModbusMode>(
        settings.value("connection/mode", static_cast<int>(ModbusMode::Tcp)).toInt()
    );

    savedDeviceConfig.tcp.host = settings.value("connection/tcpHost", "127.0.0.1").toString();
    savedDeviceConfig.tcp.port = settings.value("connection/tcpPort", 5020).toInt();
    savedDeviceConfig.serial.portName = settings.value("connection/serialPort", "COM13").toString();
    savedDeviceConfig.serial.baudRate = settings.value("connection/baudRate", 9600).toInt();
    savedDeviceConfig.slaveId = settings.value("connection/slaveId", 1).toInt();

    connectionPanel->setInitialConfig(savedDeviceConfig);

    pollingIntervalMs = settings.value("polling/intervalMs", 1000).toInt();
    pollingStartAddress = settings.value("polling/startAddress", 0).toInt();
    pollingCount = settings.value("polling/count", 4).toInt();

    // 轮询配置面板负责显示和修改轮询参数，修改后发出信号通知主窗口更新配置并持久化。
    PollingConfigPanel *pollingConfigPanel = new PollingConfigPanel(tabs);
    pollingConfigPanel->setInitialConfig(
        pollingIntervalMs,
        pollingStartAddress,
        pollingCount
    );

    const double savedTemperatureHighLimit =
        settings.value(
            "界限/最高温度",
            alarmManager->temperatureHighLimitValue()
        ).toDouble();

    const double savedVoltageLowLimit =
        settings.value(
            "界限/最低电压",
            alarmManager->voltageLowLimitValue()
        ).toDouble();

    alarmManager->setAlarmLimits(savedTemperatureHighLimit, savedVoltageLowLimit);

    AlarmConfigPanel *alarmConfigPanel = new AlarmConfigPanel(tabs);

    alarmConfigPanel->setInitialLimits(
        savedTemperatureHighLimit,
        savedVoltageLowLimit
    );

    QTabWidget *historyTabs = new QTabWidget(tabs);
    historyTabs->addTab(historyPanel, "采集数据日志");
    historyTabs->addTab(alarmHistoryPanel, "报警日志");
    historyTabs->addTab(packetHistoryPanel, "报文日志");
    tabs->addTab(connectionPanel, "设备连接");
    tabs->addTab(registerPanel, "寄存器调试");
    tabs->addTab(monitorPanel, "实时监控");
    tabs->addTab(trendPanel, "实时曲线");
    tabs->addTab(pollingConfigPanel, "采集配置");
    tabs->addTab(alarmPanel, "报警记录");
    tabs->addTab(alarmConfigPanel, "报警配置");
    tabs->addTab(historyTabs, "历史查询");
    tabs->addTab(databaseMaintenancePanel, "数据库维护");
    tabs->addTab(packetPanel, "报文日志");

    // 数据库只打开和初始化一次，采集数据与报警记录共用同一个数据库连接。
    if (databaseManager->open("modbus_hmi.db") && databaseManager->initialize()) {
        connect(pollingWorker, &PollingWorker::engineeringValueReady,
            databaseManager, &DatabaseManager::saveEngineeringValue
        );

        connect(alarmManager, &AlarmManager::alarmRaised,
            databaseManager, &DatabaseManager::saveAlarmRecord
        );

        connect(alarmPanel, &AlarmPanel::alarmConfirmed,
            databaseManager, &DatabaseManager::confirmAlarm
        );

        databaseMaintenancePanel->refreshInfo();
    }

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
        modbusClient, &QtModbusClient::connectDevice
    );

    connect(connectionPanel, &ConnectionPanel::disconnectRequested,
        modbusClient, &QtModbusClient::disconnectDevice
    );

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

    connect(connectionPanel, &ConnectionPanel::connectRequested,
    this, [](const DeviceConfig &config) {
        QSettings settings("QtModbusHmi", "ModbusIndustrialHmi");

        settings.setValue("connection/mode", static_cast<int>(config.mode));
        settings.setValue("connection/tcpHost", config.tcp.host);
        settings.setValue("connection/tcpPort", config.tcp.port);
        settings.setValue("connection/serialPort", config.serial.portName);
        settings.setValue("connection/baudRate", config.serial.baudRate);
        settings.setValue("connection/slaveId", config.slaveId);
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
        modbusClient, &QtModbusClient::readHoldingRegisters
    );

    connect(registerPanel, &RegisterPanel::writeSingleHoldingRegisterRequested,
        modbusClient, &QtModbusClient::writeSingleHoldingRegister
    );

    connect(modbusClient, &QtModbusClient::holdingRegistersRead,
        registerPanel, &RegisterPanel::displayHoldingRegisters
    );

    // 连接成功后启动周期轮询；断开后停止轮询，避免继续发送读请求。
    connect(modbusClient, &QtModbusClient::connected,
        this, [this, pollingWorker]() {
            modbusConnected = true;
            pollingWorker->start(
                pollingIntervalMs,
                pollingStartAddress,
                pollingCount
            );
    });

    connect(modbusClient, &QtModbusClient::disconnected,
    this, [this, pollingWorker]() {
        modbusConnected = false;
        pollingWorker->stop();
    });

    connect(pollingWorker, &PollingWorker::readRequested,
        modbusClient, &QtModbusClient::readHoldingRegisters
    );

    connect(modbusClient, &QtModbusClient::holdingRegistersRead,
        pollingWorker, &PollingWorker::onRegistersRead
    );

    connect(modbusClient, &QtModbusClient::errorOccurred,
        pollingWorker, &PollingWorker::onError
    );

    // 轮询配置更改后，立即应用新配置并持久化到磁盘，重启轮询以应用新参数。
    connect(pollingConfigPanel, &PollingConfigPanel::pollingConfigChanged,
    this, [this, pollingWorker](int intervalMs, int startAddress, int count) {
        pollingIntervalMs = intervalMs;
        pollingStartAddress = startAddress;
        pollingCount = count;

        QSettings settings("QtModbusHmi", "ModbusIndustrialHmi");
        settings.setValue("polling/intervalMs", pollingIntervalMs);
        settings.setValue("polling/startAddress", pollingStartAddress);
        settings.setValue("polling/count", pollingCount);

        if (modbusConnected) {
            pollingWorker->start(
                pollingIntervalMs,
                pollingStartAddress,
                pollingCount
            );
        }

        statusBar()->showMessage(
            QString("Polling config saved: interval=%1 ms, start=%2, count=%3")
                .arg(pollingIntervalMs)
                .arg(pollingStartAddress)
                .arg(pollingCount)
        );
    });

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
        alarmManager, &AlarmManager::onDeviceOffline
    );

    connect(modbusClient, &QtModbusClient::unexpectedDisconnected,
        alarmManager, &AlarmManager::onDeviceOfflineForDevice
    );

    connect(pollingWorker, &PollingWorker::deviceOffline,
        this, [this]() {
            statusBar()->showMessage("设备连续通信失败，判定离线");
    });

    connect(alarmManager, &AlarmManager::alarmRaised,
        alarmPanel, &AlarmPanel::appendAlarm
    );

    connect(alarmManager, &AlarmManager::alarmRaised,
        this, [this](const AlarmRecord &alarm) {
            statusBar()->showMessage("报警：" + alarm.message);
    });

    // 报文日志用于观察用户操作、通信结果、报警和数据库错误。
    connect(connectionPanel, &ConnectionPanel::connectRequested,
    packetPanel, [appendPersistentLog](const DeviceConfig &config) {
        appendPersistentLog(
            "Connection",
            "TX",
            QString("Connect request %1:%2 slave=%3")
                .arg(config.tcp.host)
                .arg(config.tcp.port)
                .arg(config.slaveId)
        );
    });

    connect(connectionPanel, &ConnectionPanel::disconnectRequested,
    packetPanel, [appendPersistentLog]() {
        appendPersistentLog(
            "Connection",
            "TX",
            "Disconnect request"
        );
    });

    connect(registerPanel, &RegisterPanel::readHoldingRegistersRequested,
        packetPanel, [appendPersistentLog](int startAddress, int count) {
        appendPersistentLog(
            "Modbus",
            "TX",
            QString("Read holding registers start=%1 count=%2")
                .arg(startAddress)
                .arg(count)
        );
    });


    connect(registerPanel, &RegisterPanel::writeSingleHoldingRegisterRequested,
        packetPanel, [appendPersistentLog](int address, quint16 value) {
            appendPersistentLog(
                "Modbus",
                "TX",
                QString("Write single holding register address=%1 value=%2")
                    .arg(address)
                    .arg(value)
            );
    });

    connect(modbusClient, &QtModbusClient::errorOccurred,
        packetPanel, [appendPersistentLog](const QString &message) {
            appendPersistentLog(
                "Communication",
                "RX",
                "Error " + message
            );
    });

    connect(modbusClient, &QtModbusClient::holdingRegistersRead,
        packetPanel, [appendPersistentLog](const RegisterReadResult &result) {
            appendPersistentLog(
                "Modbus",
                "RX",
                QString("Holding registers read ok start=%1 count=%2")
                    .arg(result.startAddress)
                    .arg(result.values.size())
            );
    });

    connect(alarmManager, &AlarmManager::alarmRaised,
        packetPanel, [appendPersistentLog](const AlarmRecord &alarm) {
            appendPersistentLog(
                "Alarm",
                "ALARM",
                alarm.message
            );
    });


    connect(alarmConfigPanel, &AlarmConfigPanel::alarmLimitsChanged,
        alarmManager, &AlarmManager::setAlarmLimits
    );

    connect(alarmConfigPanel, &AlarmConfigPanel::alarmLimitsChanged,
    this, [this](double temperatureHighLimit, double voltageLowLimit) {
        QSettings settings("QtModbusHmi", "ModbusIndustrialHmi");
        settings.setValue("界限/最高温度", temperatureHighLimit);
        settings.setValue("界限/最低电压", voltageLowLimit);

        statusBar()->showMessage(
            QString("界限更改为: 温度 > %1, 电压 < %2")
                .arg(temperatureHighLimit)
                .arg(voltageLowLimit)
        );
    });

    connect(pollingWorker, &PollingWorker::engineeringValueReady,
        trendPanel, &TrendPanel::appendValue
    );

    setCentralWidget(tabs);
}
