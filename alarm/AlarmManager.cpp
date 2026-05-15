#include "AlarmManager.h"

#include <QUuid>

AlarmManager::AlarmManager(QObject *parent)
    : QObject(parent)
{
}

void AlarmManager::checkValue(const EngineeringValue &value)
{
    // 越限只在“正常 -> 报警”边沿触发一次；恢复正常后才允许下一次触发。
    if (value.temperature > temperatureHighLimit) {
        if (!temperatureHighActive) {
            temperatureHighActive = true;

            emit alarmRaised(createAlarm(
                value.deviceId,
                AlarmType::TemperatureHigh,
                AlarmLevel::Critical,
                value.temperature,
                temperatureHighLimit,
                "温度超过上限"
            ));
        }
    } else {
        temperatureHighActive = false;
    }

    if (value.voltage < voltageLowLimit) {
        if (!voltageLowActive) {
            voltageLowActive = true;

            emit alarmRaised(createAlarm(
                value.deviceId,
                AlarmType::VoltageLow,
                AlarmLevel::Warning,
                value.voltage,
                voltageLowLimit,
                "电压低于下限"
            ));
        }
    } else {
        voltageLowActive = false;
    }

    deviceOfflineActive = false;
}

void AlarmManager::onDeviceOffline()
{
    onDeviceOfflineForDevice("device-001");
}

void AlarmManager::onDeviceOfflineForDevice(const QString &deviceId)
{
    // 离线报警同样做去重，避免连续通信失败时刷屏。
    if (deviceOfflineActive) {
        return;
    }

    deviceOfflineActive = true;

    emit alarmRaised(createAlarm(
        deviceId,
        AlarmType::DeviceOffline,
        AlarmLevel::Critical,
        0.0,
        0.0,
        "设备连续通信失败，判定离线"
    ));
}

AlarmRecord AlarmManager::createAlarm(
    const QString &deviceId,
    AlarmType type,
    AlarmLevel level,
    double currentValue,
    double thresholdValue,
    const QString &message
) const
{
    AlarmRecord alarm;
    alarm.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    alarm.deviceId = deviceId;
    alarm.type = type;
    alarm.level = level;
    alarm.currentValue = currentValue;
    alarm.thresholdValue = thresholdValue;
    alarm.message = message;
    alarm.confirmed = false;
    alarm.alarmTime = QDateTime::currentDateTime();

    return alarm;
}

double AlarmManager::temperatureHighLimitValue() const
{
    return temperatureHighLimit;
}

double AlarmManager::voltageLowLimitValue() const
{
    return voltageLowLimit;
}

void AlarmManager::setAlarmLimits(double newTemperatureHighLimit, double newVoltageLowLimit)
{
    // 阈值变更后重置激活状态，让新阈值可以立即重新评估报警边沿。
    temperatureHighLimit = newTemperatureHighLimit;
    voltageLowLimit = newVoltageLowLimit;

    temperatureHighActive = false;
    voltageLowActive = false;
}

