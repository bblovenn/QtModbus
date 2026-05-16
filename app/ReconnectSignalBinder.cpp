#include "ReconnectSignalBinder.h"

#include "AppSettings.h"
#include "PacketLogService.h"
#include "../communication/QtModbusClient.h"
#include "../communication/ReconnectController.h"
#include "../ui/ConnectionPanel.h"
#include "../ui/PacketMonitorPanel.h"
#include "../ui/ReconnectConfigPanel.h"
#include "../ui/SystemStatusPanel.h"

#include <QMainWindow>
#include <QStatusBar>

void ReconnectSignalBinder::bind(const MainWindowSignalBinderContext &context)
{
    // 自动重连域：用户主动断开会停止重连，异常断开才进入重连流程。
    QMainWindow *window = context.window;
    AppSettings *settings = context.settings;
    MainWindowPanels *panels = context.panels;
    PacketLogService *packetLog = context.packetLog;
    QtModbusClient *modbusClient = context.modbusClient;
    ReconnectController *reconnectController = context.reconnectController;

    // —— 连接时记录配置，供后续自动重连使用 ——
    QObject::connect(panels->connection, &ConnectionPanel::connectRequested,
        reconnectController, &ReconnectController::rememberConfig
    );

    // —— 用户主动断开 / 连接成功后停止重连 ——
    QObject::connect(panels->connection, &ConnectionPanel::disconnectRequested,
        reconnectController, &ReconnectController::stopReconnect
    );

    QObject::connect(modbusClient, &QtModbusClient::connected,
        reconnectController, &ReconnectController::stopReconnect
    );

    // —— 异常断开 → 启动重连流程 ——
    QObject::connect(modbusClient, &QtModbusClient::unexpectedDisconnected,
        reconnectController, &ReconnectController::scheduleReconnect
    );

    // —— 重连请求 → 通信层执行连接 ——
    QObject::connect(reconnectController, &ReconnectController::reconnectRequested,
        modbusClient, &QtModbusClient::connectDevice
    );

    // —— 重连消息 → 状态栏 + 报文日志 ——
    QObject::connect(reconnectController, &ReconnectController::reconnectMessage,
        window, [window](const QString &message) {
            window->statusBar()->showMessage(message);
    });

    QObject::connect(reconnectController, &ReconnectController::reconnectMessage,
        panels->packets, [packetLog](const QString &message) {
            packetLog->append("Connection", "自动", message);
    });

    // —— 重连中状态 → 状态总览 ——
    QObject::connect(reconnectController, &ReconnectController::reconnectRequested,
        panels->systemStatus, &SystemStatusPanel::setConnecting
    );

    // —— 重连配置变更 → 控制器 + 持久化 ——
    QObject::connect(panels->reconnectConfig, &ReconnectConfigPanel::reconnectConfigChanged,
        window, [window, settings, reconnectController](bool enabled, int intervalMs) {
            reconnectController->setEnabled(enabled);
            reconnectController->setReconnectIntervalMs(intervalMs);
            settings->saveReconnectConfig({enabled, intervalMs});

            window->statusBar()->showMessage(
                QString("重连配置已保存：%1,间隔 %2 ms")
                    .arg(enabled ? "启用" : "禁用")
                    .arg(intervalMs)
            );
    });
}
