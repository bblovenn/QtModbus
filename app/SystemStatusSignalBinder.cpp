#include "SystemStatusSignalBinder.h"

#include "../alarm/AlarmManager.h"
#include "../communication/QtModbusClient.h"
#include "../polling/PollingWorker.h"
#include "../ui/ConnectionPanel.h"
#include "../ui/SystemStatusPanel.h"

void SystemStatusSignalBinder::bind(const MainWindowSignalBinderContext &context)
{
    // 状态总览只订阅各模块状态，不反向控制业务流程。
    MainWindowPanels *panels = context.panels;
    QtModbusClient *modbusClient = context.modbusClient;
    PollingWorker *pollingWorker = context.pollingWorker;
    AlarmManager *alarmManager = context.alarmManager;

    // —— 连接状态 ——
    QObject::connect(panels->connection, &ConnectionPanel::connectRequested,
        panels->systemStatus, &SystemStatusPanel::setConnecting
    );

    QObject::connect(panels->connection, &ConnectionPanel::disconnectRequested,
        panels->systemStatus, &SystemStatusPanel::setDisconnected
    );

    QObject::connect(modbusClient, &QtModbusClient::connected,
        panels->systemStatus, &SystemStatusPanel::setConnected
    );

    QObject::connect(modbusClient, &QtModbusClient::disconnected,
        panels->systemStatus, &SystemStatusPanel::setDisconnected
    );

    QObject::connect(modbusClient, &QtModbusClient::errorOccurred,
        panels->systemStatus, &SystemStatusPanel::setCommunicationError
    );

    // —— 轮询状态 ——
    QObject::connect(pollingWorker, &PollingWorker::pollingStarted,
        panels->systemStatus, &SystemStatusPanel::setPollingStarted
    );

    QObject::connect(pollingWorker, &PollingWorker::pollingStopped,
        panels->systemStatus, &SystemStatusPanel::setPollingStopped
    );

    // —— 最新工程值 ——
    QObject::connect(pollingWorker, &PollingWorker::engineeringValueReady,
        panels->systemStatus, &SystemStatusPanel::updateEngineeringValue
    );

    // —— 最新报警 ——
    QObject::connect(alarmManager, &AlarmManager::alarmRaised,
        panels->systemStatus, &SystemStatusPanel::updateAlarm
    );
}
