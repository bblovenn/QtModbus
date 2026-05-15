#ifndef RECONNECTCONTROLLER_H
#define RECONNECTCONTROLLER_H

#include "../core/DeviceConfig.h"

#include <QObject>

class QTimer;

// 自动重连控制器：记录最近一次连接配置，在异常断开后按间隔重新发起连接请求。
class ReconnectController : public QObject
{
    Q_OBJECT

public:
    explicit ReconnectController(QObject *parent = nullptr);
    
    bool isEnabled() const;
    int reconnectIntervalMs() const;

public slots:
    void rememberConfig(const DeviceConfig &config);
    void scheduleReconnect(const QString &deviceId);
    void stopReconnect();
    void setEnabled(bool enabled);
    void setReconnectIntervalMs(int intervalMs);

signals:
    void reconnectRequested(const DeviceConfig &config);
    void reconnectMessage(const QString &message);

private slots:
    void doReconnect();

private:
    QTimer *timer = nullptr;
    DeviceConfig lastConfig;
    bool hasConfig = false;
    bool enabled = true;
    int attempt = 0;
    int intervalMs = 3000;
};

#endif
