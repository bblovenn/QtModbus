#ifndef POLLINGWORKER_H
#define POLLINGWORKER_H

#include "../core/EngineeringValue.h"
#include "../core/RegisterValue.h"

#include <QObject>

//轮询读取Modbus寄存器

class QTimer;

class PollingWorker : public QObject
{
    Q_OBJECT

public:
    explicit PollingWorker(QObject *parent = nullptr);

public slots:
    void start(int intervalMs, int startAddress, int count); // 启动轮询
    void stop();
    void onRegistersRead(const RegisterReadResult &result); // 处理读取结果
    void onError(const QString &message);

signals:
    void readRequested(int startAddress, int count);  // 请求读取
    void engineeringValueReady(const EngineeringValue &value);  // 工程值就绪
    void deviceOffline(); // 设备离线
    void pollingStarted();
    void pollingStopped();

private:
    EngineeringValue convertRegisters(const RegisterReadResult &result) const;// 原始值转工程值

private:
    QTimer *timer = nullptr;
    int startAddress = 0;
    int count = 4; // 读取数量
    int failedCount = 0; //连续失败次数
    int maxFailedCount = 3;
};

#endif // POLLINGWORKER_H