#include "AlarmManager.h"

#include <QUuid>

AlarmManager::AlarmManager(QObject *parent)
    : QObject(parent)
{
}

void AlarmManager::checkValue(const EngineeringValue &value)
{
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
    temperatureHighLimit = newTemperatureHighLimit;
    voltageLowLimit = newVoltageLowLimit;

    temperatureHighActive = false;
    voltageLowActive = false;
}

