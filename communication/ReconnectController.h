#ifndef RECONNECTCONTROLLER_H
#define RECONNECTCONTROLLER_H

#include "../core/DeviceConfig.h"

#include <QObject>

class QTimer;

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
