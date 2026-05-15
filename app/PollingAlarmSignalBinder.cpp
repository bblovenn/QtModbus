#include "PollingAlarmSignalBinder.h"

#include "AppSettings.h"
#include "../alarm/AlarmManager.h"
#include "../communication/QtModbusClient.h"
#include "../core/AlarmRecord.h"
#include "../core/EngineeringValue.h"
#include "../polling/PollingWorker.h"
#include "../ui/AlarmConfigPanel.h"
#include "../ui/AlarmPanel.h"
#include "../ui/MonitorPanel.h"
#include "../ui/PollingConfigPanel.h"
#include "../ui/TrendPanel.h"

#include <QMainWindow>
#include <QStatusBar>

void PollingAlarmSignalBinder::bind(const MainWindowSignalBinderContext &context)
{
    // 采集报警域：连接成功后启动轮询，轮询结果同时驱动监控、趋势和报警判断。
    QMainWindow *window = context.window;
    AppSettings *settings = context.settings;
    MainWindowPanels *panels = context.panels;
    MainWindowRuntimeState *runtime = context.runtime;
    QtModbusClient *modbusClient = context.modbusClient;
    PollingWorker *pollingWorker = context.pollingWorker;
    AlarmManager *alarmManager = context.alarmManager;

    // —— 连接成功 → 自动启动轮询；断开 → 停止轮询 ——
    QObject::connect(modbusClient, &QtModbusClient::connected,
        window, [runtime, pollingWorker]() {
            runtime->modbusConnected = true;
            pollingWorker->start(
                runtime->pollingIntervalMs,
                runtime->pollingStartAddress,
                runtime->pollingCount
            );
    });

    QObject::connect(modbusClient, &QtModbusClient::disconnected,
        window, [runtime, pollingWorker]() {
            runtime->modbusConnected = false;
            pollingWorker->stop();
    });

    // —— 轮询读取请求 → 通信层 ——
    QObject::connect(pollingWorker, &PollingWorker::readRequested,
        modbusClient, &QtModbusClient::readHoldingRegisters
    );

    // —— 通信结果 → 轮询处理器 ——
    QObject::connect(modbusClient, &QtModbusClient::holdingRegistersRead,
        pollingWorker, &PollingWorker::onRegistersRead
    );

    QObject::connect(modbusClient, &QtModbusClient::errorOccurred,
        pollingWorker, &PollingWorker::onError
    );

    QObject::connect(panels->pollingConfig, &PollingConfigPanel::pollingConfigChanged,
        window, [window, settings, runtime, pollingWorker](int intervalMs, int startAddress, int count) {
            runtime->pollingIntervalMs = intervalMs;
            runtime->pollingStartAddress = startAddress;
            runtime->pollingCount = count;

            settings->savePollingConfig({intervalMs, startAddress, count});

            if (runtime->modbusConnected) {
                pollingWorker->start(
                    runtime->pollingIntervalMs,
                    runtime->pollingStartAddress,
                    runtime->pollingCount
                );
            }

            window->statusBar()->showMessage(
                QString("Polling config saved: interval=%1 ms, start=%2, count=%3")
                    .arg(runtime->pollingIntervalMs)
                    .arg(runtime->pollingStartAddress)
                    .arg(runtime->pollingCount)
            );
    });

    // —— 工程值分发：同时驱动监控面板、报警判断和状态栏 ——
    QObject::connect(pollingWorker, &PollingWorker::engineeringValueReady,
        panels->monitor, &MonitorPanel::updateValue);

    QObject::connect(pollingWorker, &PollingWorker::engineeringValueReady,
        alarmManager, &AlarmManager::checkValue);

    QObject::connect(pollingWorker, &PollingWorker::engineeringValueReady,
        window, [window](const EngineeringValue &value) {
            window->statusBar()->showMessage(
                QString("温度=%1 ℃  电压=%2 V  电流=%3 A  转速=%4 rpm")
                    .arg(value.temperature)
                    .arg(value.voltage)
                    .arg(value.current)
                    .arg(value.speed)
            );
    });

    // —— 设备离线 → 报警 ——
    QObject::connect(pollingWorker, &PollingWorker::deviceOffline,
        alarmManager, &AlarmManager::onDeviceOffline
    );

    QObject::connect(modbusClient, &QtModbusClient::unexpectedDisconnected,
        alarmManager, &AlarmManager::onDeviceOfflineForDevice
    );

    QObject::connect(pollingWorker, &PollingWorker::deviceOffline,
        window, [window]() {
            window->statusBar()->showMessage("设备连续通信失败，判定离线");
    });

    // —— 报警 → 实时报警面板 + 状态栏 ——
    QObject::connect(alarmManager, &AlarmManager::alarmRaised,
        panels->alarms, &AlarmPanel::appendAlarm
    );

    QObject::connect(alarmManager, &AlarmManager::alarmRaised,
        window, [window](const AlarmRecord &alarm) {
            window->statusBar()->showMessage("报警：" + alarm.message);
    });

    // —— 报警阈值配置变更 → 报警管理器 + 持久化 ——
    QObject::connect(panels->alarmConfig, &AlarmConfigPanel::alarmLimitsChanged,
        alarmManager, &AlarmManager::setAlarmLimits
    );

    QObject::connect(panels->alarmConfig, &AlarmConfigPanel::alarmLimitsChanged,
        window, [window, settings](double temperatureHighLimit, double voltageLowLimit) {
            settings->saveAlarmLimits({temperatureHighLimit, voltageLowLimit});

            window->statusBar()->showMessage(
                QString("界限更改为: 温度 > %1, 电压 < %2")
                    .arg(temperatureHighLimit)
                    .arg(voltageLowLimit)
            );
    });

    // —— 工程值 → 实时曲线 ——
    QObject::connect(pollingWorker, &PollingWorker::engineeringValueReady,
        panels->trend, &TrendPanel::appendValue
    );
}
