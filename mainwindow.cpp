#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "./ui/ConnectionPanel.h"
#include "./communication/QtModbusClient.h"
#include "./ui/RegisterPanel.h"
#include "./ui/PacketMonitorPanel.h"
#include "./core/PacketRecord.h"
#include "./polling/PollingWorker.h"
#include "./ui/MonitorPanel.h"


#include <QLabel>
#include <QString>
#include <QStatusBar>
#include <QTabWidget>
#include <QDateTime>

namespace {
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setupMainTabs();

    resize(1200, 800);
    setWindowTitle(zh(u8"Qt Modbus RTU/TCP \u5DE5\u4E1A\u8BBE\u5907\u76D1\u63A7\u4E0E\u8C03\u8BD5\u7CFB\u7EDF"));
    statusBar()->showMessage(zh(u8"\u672A\u8FDE\u63A5"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupMainTabs()
{
    // 创建主标签页
    tabs = new QTabWidget(this);

    // 设备连接面板
    ConnectionPanel *connectionPanel = new ConnectionPanel(tabs);
    tabs->addTab(connectionPanel, "设备连接");

    connect(connectionPanel, &ConnectionPanel::connectRequested,
        this, [this](const DeviceConfig &config) {
            statusBar()->showMessage("请求连接：" + config.tcp.host + ":" + QString::number(config.tcp.port));
    });

    connect(connectionPanel, &ConnectionPanel::disconnectRequested,
        this, [this]() {
            statusBar()->showMessage("已断开");
    });

    // 创建 Modbus 客户端实例（实际连接在 ConnectionPanel 中触发）
    QtModbusClient *modbusClient = new QtModbusClient(this);

    connect(connectionPanel, &ConnectionPanel::connectRequested,
        modbusClient, &QtModbusClient::connectDevice);

    connect(connectionPanel, &ConnectionPanel::disconnectRequested,
        modbusClient, &QtModbusClient::disconnectDevice);

    connect(modbusClient, &QtModbusClient::connected, this, [this]() {
        statusBar()->showMessage("已连接");
    });

    connect(modbusClient, &QtModbusClient::disconnected, this, [this]() {
        statusBar()->showMessage("已断开");
    });

    connect(modbusClient, &QtModbusClient::errorOccurred, this, [this](const QString &message) {
        statusBar()->showMessage("通信错误：" + message);
    });
 
    RegisterPanel *registerPanel = new RegisterPanel(tabs);
    tabs->addTab(registerPanel, "寄存器调试");

    connect(registerPanel, &RegisterPanel::readHoldingRegistersRequested,
        modbusClient, &QtModbusClient::readHoldingRegisters
    );

    connect(registerPanel, &RegisterPanel::writeSingleHoldingRegisterRequested,
        modbusClient, &QtModbusClient::writeSingleHoldingRegister
    );

    connect(modbusClient, &QtModbusClient::holdingRegistersRead,
        registerPanel, &RegisterPanel::displayHoldingRegisters
    );

    MonitorPanel *monitorPanel = new MonitorPanel(tabs);
    tabs->addTab(monitorPanel, "实时监控");

    PollingWorker *pollingWorker = new PollingWorker(this);

    connect(pollingWorker, &PollingWorker::engineeringValueReady,
        monitorPanel, &MonitorPanel::updateValue
    );

    tabs->addTab(new QLabel("报警记录面板将在后续任务实现", tabs), "报警记录");
    tabs->addTab(new QLabel("历史数据查询将在后续任务实现", tabs), "历史查询");  

    connect(modbusClient, &QtModbusClient::connected,
        pollingWorker, [pollingWorker]() {
            pollingWorker->start(1000, 0, 4);
        }
    );

    connect(modbusClient, &QtModbusClient::disconnected,
        pollingWorker, &PollingWorker::stop
    );

    connect(pollingWorker, &PollingWorker::readRequested,
        modbusClient, &QtModbusClient::readHoldingRegisters
    );

    connect(modbusClient, &QtModbusClient::holdingRegistersRead,
        pollingWorker, &PollingWorker::onRegistersRead
    );

    connect(modbusClient, &QtModbusClient::errorOccurred,
        pollingWorker, &PollingWorker::onError
    );

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

    connect(pollingWorker, &PollingWorker::deviceOffline,
        this, [this]() {
            statusBar()->showMessage("设备连续通信失败，判定离线");
    });


    PacketMonitorPanel *packetPanel = new PacketMonitorPanel(tabs);
    tabs->addTab(packetPanel, "报文日志");

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

    //读取寄存器
    connect(registerPanel, &RegisterPanel::readHoldingRegistersRequested,
        packetPanel, [packetPanel](int startAddress, int count) {
            packetPanel->appendText(
                QString("[%1] TX: Read holding registers start=%2 count=%3")
                    .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"))
                    .arg(startAddress)
                    .arg(count)
            );
        });
    //写入寄存器
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

    setCentralWidget(tabs);
}
