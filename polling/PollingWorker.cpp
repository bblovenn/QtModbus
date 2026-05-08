#include "PollingWorker.h"

#include <QTimer>

PollingWorker::PollingWorker(QObject *parent)
    : QObject(parent)
{
    timer = new QTimer(this);

    connect(timer, &QTimer::timeout, this, [this]() {
        emit readRequested(startAddress, count);
    });
}

void PollingWorker::start(int intervalMs, int startAddressValue, int countValue)
{
    startAddress = startAddressValue;
    count = countValue;
    failedCount = 0;

    timer->start(intervalMs);
    emit pollingStarted();

    emit readRequested(startAddress, count); // 立即发射一次读取请求（不等定时器）
}

void PollingWorker::stop()
{
    timer->stop();
    emit pollingStopped();
}

//处理读取成功的相应

void PollingWorker::onRegistersRead(const RegisterReadResult &result)
{
    failedCount = 0;

    EngineeringValue value = convertRegisters(result); // 原始寄存器值 → 工程值
    emit engineeringValueReady(value);
}

//maxFailedCount 防止网络抖动导致的误判
void PollingWorker::onError(const QString &message)
{
    Q_UNUSED(message)

    ++failedCount;

    if (failedCount >= maxFailedCount) {
        emit deviceOffline();
    }
}

//原始寄存器值转换为工程值
EngineeringValue PollingWorker::convertRegisters(const RegisterReadResult &result) const
{
    EngineeringValue value;
    value.deviceId = result.deviceId;
    value.timestamp = result.timestamp;

    if (result.values.size() >= 4) { // 确保至少有4个寄存器值
        value.temperature = result.values.at(0).rawValue / 10.0;
        value.voltage = result.values.at(1).rawValue / 100.0;
        value.current = result.values.at(2).rawValue / 100.0;
        value.speed = result.values.at(3).rawValue;
    }

    return value;
}
