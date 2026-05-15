#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "app/MainWindowSignalBinder.h"
#include "app/PacketLogService.h"
#include "alarm/AlarmManager.h"
#include "communication/QtModbusClient.h"
#include "communication/ReconnectController.h"
#include "database/DatabaseManager.h"
#include "polling/PollingWorker.h"
#include "ui/AlarmConfigPanel.h"
#include "ui/AlarmHistoryPanel.h"
#include "ui/AlarmPanel.h"
#include "ui/ConnectionPanel.h"
#include "ui/DatabaseMaintenancePanel.h"
#include "ui/HistoryPanel.h"
#include "ui/HistoryTrendPanel.h"
#include "ui/MonitorPanel.h"
#include "ui/PacketHistoryPanel.h"
#include "ui/PacketMonitorPanel.h"
#include "ui/PollingConfigPanel.h"
#include "ui/ReconnectConfigPanel.h"
#include "ui/RegisterPanel.h"
#include "ui/SystemStatusPanel.h"
#include "ui/TrendPanel.h"

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
    // 初始化顺序：历史类面板依赖 databaseManager，信号绑定依赖所有面板和服务都已创建。
    createServices();
    createPanels();
    loadInitialSettings();
    addTabs();
    initializeDatabase();
    connectSignals();

    setCentralWidget(panels.mainTabs);
}

void MainWindow::createServices()
{
    // 业务对象由主窗口持有，随主窗口生命周期一起释放。
    modbusClient = new QtModbusClient(this);
    pollingWorker = new PollingWorker(this);
    alarmManager = new AlarmManager(this);
    databaseManager = new DatabaseManager(this);
    reconnectController = new ReconnectController(this);
}

void MainWindow::createPanels()
{
    // 面板对象统一挂到 mainTabs 下，由 Qt parent-child 机制负责释放。
    panels.mainTabs = new QTabWidget(this);

    panels.connection = new ConnectionPanel(panels.mainTabs);
    panels.registers = new RegisterPanel(panels.mainTabs);
    panels.monitor = new MonitorPanel(panels.mainTabs);
    panels.alarms = new AlarmPanel(panels.mainTabs);
    panels.packets = new PacketMonitorPanel(panels.mainTabs); // 实时报文监控面板
    panels.trend = new TrendPanel(panels.mainTabs); // 实时曲线面板
    panels.systemStatus = new SystemStatusPanel(panels.mainTabs);
    panels.reconnectConfig = new ReconnectConfigPanel(panels.mainTabs);
    panels.history = new HistoryPanel(databaseManager, panels.mainTabs);
    panels.alarmHistory = new AlarmHistoryPanel(databaseManager, panels.mainTabs);
    panels.historyTrend = new HistoryTrendPanel(databaseManager, panels.mainTabs);
    panels.packetHistory = new PacketHistoryPanel(databaseManager, panels.mainTabs);
    panels.databaseMaintenance = new DatabaseMaintenancePanel(databaseManager, panels.mainTabs);
    panels.pollingConfig = new PollingConfigPanel(panels.mainTabs); // 轮询配置面板
    panels.alarmConfig = new AlarmConfigPanel(panels.mainTabs);

    panels.historyTabs = new QTabWidget(panels.mainTabs); // 历史查询的二级标签页
    packetLogService = std::make_unique<PacketLogService>(panels.packets, databaseManager); // 创建报文日志服务，传入实时报文监控面板和数据库管理器的指针
}

void MainWindow::loadInitialSettings()
{
    // 启动时恢复用户上次保存的连接、轮询、报警和重连配置。
    panels.connection->setInitialConfig(appSettings.loadDeviceConfig());

    const PollingConfig pollingConfig = appSettings.loadPollingConfig();
    runtimeState.pollingIntervalMs = pollingConfig.intervalMs;
    runtimeState.pollingStartAddress = pollingConfig.startAddress;
    runtimeState.pollingCount = pollingConfig.count;
    panels.pollingConfig->setInitialConfig(
        runtimeState.pollingIntervalMs,
        runtimeState.pollingStartAddress,
        runtimeState.pollingCount
    );

    const AlarmLimits alarmLimits = appSettings.loadAlarmLimits(
        alarmManager->temperatureHighLimitValue(),
        alarmManager->voltageLowLimitValue()
    );
    alarmManager->setAlarmLimits(
        alarmLimits.temperatureHighLimit,
        alarmLimits.voltageLowLimit
    );
    panels.alarmConfig->setInitialLimits(
        alarmLimits.temperatureHighLimit,
        alarmLimits.voltageLowLimit
    );

    const ReconnectConfig reconnectConfig = appSettings.loadReconnectConfig(
        reconnectController->isEnabled(),
        reconnectController->reconnectIntervalMs()
    );
    reconnectController->setEnabled(reconnectConfig.enabled);
    reconnectController->setReconnectIntervalMs(reconnectConfig.intervalMs);
    panels.reconnectConfig->setInitialConfig(
        reconnectConfig.enabled,
        reconnectConfig.intervalMs
    );
}

void MainWindow::addTabs()
{
    panels.historyTabs->addTab(panels.history, "采集数据日志");
    panels.historyTabs->addTab(panels.historyTrend, "历史曲线");
    panels.historyTabs->addTab(panels.alarmHistory, "报警日志");
    panels.historyTabs->addTab(panels.packetHistory, "报文日志");

    panels.mainTabs->addTab(panels.connection, "设备连接");
    panels.mainTabs->addTab(panels.systemStatus, "状态总览");
    panels.mainTabs->addTab(panels.registers, "寄存器调试");
    panels.mainTabs->addTab(panels.monitor, "实时监控");
    panels.mainTabs->addTab(panels.trend, "实时曲线");
    panels.mainTabs->addTab(panels.pollingConfig, "采集配置");
    panels.mainTabs->addTab(panels.reconnectConfig, "重连配置");
    panels.mainTabs->addTab(panels.alarms, "报警记录");
    panels.mainTabs->addTab(panels.alarmConfig, "报警配置");
    panels.mainTabs->addTab(panels.historyTabs, "历史查询");
    panels.mainTabs->addTab(panels.databaseMaintenance, "数据库维护");
    panels.mainTabs->addTab(panels.packets, "报文日志");
}

void MainWindow::initializeDatabase()
{
    // 数据库初始化失败时只禁用持久化能力，界面和通信调试仍可继续使用。
    if (!databaseManager->open("modbus_hmi.db") || !databaseManager->initialize()) {
        return;
    }

    connect(pollingWorker, &PollingWorker::engineeringValueReady,
        databaseManager, &DatabaseManager::saveEngineeringValue
    );

    connect(alarmManager, &AlarmManager::alarmRaised,
        databaseManager, &DatabaseManager::saveAlarmRecord
    );

    connect(panels.alarms, &AlarmPanel::alarmConfirmed,
        databaseManager, &DatabaseManager::confirmAlarm
    );

    panels.databaseMaintenance->refreshInfo();
    panels.systemStatus->setDatabaseReady(databaseManager->databaseFilePath());
}

void MainWindow::connectSignals()
{
    // 所有跨模块信号槽集中交给 Binder，MainWindow 保持为装配入口。
    MainWindowSignalBinder::bind({
        this,
        &appSettings,
        &panels,
        &runtimeState,
        packetLogService.get(),
        modbusClient,
        pollingWorker,
        alarmManager,
        databaseManager,
        reconnectController
    });
}
