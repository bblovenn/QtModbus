#include "PollingWorker.h"

#include <QTimer>

PollingWorker::PollingWorker(QObject *parent)
    : QObject(parent)
{
    timer = new QTimer(this);

    // 定时器只负责节拍，真正的 Modbus 读取由通信层响应 readRequested 完成。
    connect(timer, &QTimer::timeout, this, [this]() {
        emit readRequested(startAddress, count);
    });
}

void PollingWorker::start(int intervalMs, int startAddressValue, int countValue)
{
    // 重新启动轮询时清空失败计数，避免旧连接错误影响新一轮采集。
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

// 处理读取成功响应：将收到的原始寄存器值转换为工程量并发出
void PollingWorker::onRegistersRead(const RegisterReadResult &result)
{
    // 忽略非当前轮询区间的读取结果，避免手动调试读取误触发工程量刷新。
    if (result.startAddress != startAddress || result.values.size() < count) {
        return;
    }

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
    // 当前演示设备固定使用前 4 个保持寄存器：温度、电压、电流、转速。
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
