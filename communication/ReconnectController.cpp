#include "ReconnectController.h"

#include <QTimer>

ReconnectController::ReconnectController(QObject *parent)
    : QObject(parent)
{
    timer = new QTimer(this);
    timer->setInterval(intervalMs);

    connect(timer, &QTimer::timeout,
            this, &ReconnectController::doReconnect);
}

void ReconnectController::rememberConfig(const DeviceConfig &config)
{
    lastConfig = config;
    hasConfig = true;
    attempt = 0;
}

void ReconnectController::scheduleReconnect(const QString &deviceId)
{
    if (!enabled || !hasConfig) {
        return;
    }

    if (!timer->isActive()) {
        timer->start();
    }

    // 通知 UI 或日志模块：已进入自动重连流程
    emit reconnectMessage(
        QString("Device %1 disconnected, auto reconnect started")
            .arg(deviceId)
    );
}

void ReconnectController::stopReconnect()
{
    if (timer->isActive()) {
        timer->stop();
    }

    attempt = 0;
}

void ReconnectController::setEnabled(bool enabledValue)
{
    enabled = enabledValue;

    if (!enabled) {
        stopReconnect();  // 关闭时立即终止当前正在进行的重连流程
    }
}

void ReconnectController::doReconnect()
{
    if (!enabled || !hasConfig) {
        stopReconnect();
        return;
    }

    ++attempt; // 递增重试计数（第 1 次调用时为 1）

    emit reconnectMessage(
        QString("Auto reconnect attempt %1: %2:%3")
            .arg(attempt)
            .arg(lastConfig.tcp.host)
            .arg(lastConfig.tcp.port)
    );

    //发出重连请求，由上层（MainWindow）执行实际连接
    emit reconnectRequested(lastConfig);
}

bool ReconnectController::isEnabled() const
{
    return enabled;
}

int ReconnectController::reconnectIntervalMs() const
{
    return intervalMs;
}

void ReconnectController::setReconnectIntervalMs(int intervalMsValue)
{
    intervalMs = intervalMsValue;
    timer->setInterval(intervalMs);
}
