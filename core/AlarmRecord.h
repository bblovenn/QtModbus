#ifndef ALARMRECORD_H
#define ALARMRECORD_H

#include <QDateTime>
#include <QMetaType>
#include <QString>

enum class AlarmLevel{
    Info,
    Warning,
    Critical
};

enum class AlarmType{
    TemperatureHigh,
    VoltageLow,
    CommunicationTimeout,
    DeviceOffline,
    ModbusException
};

struct AlarmRecord
{
    QString id;
    QString deviceId;

    AlarmType type = AlarmType::TemperatureHigh;
    AlarmLevel level = AlarmLevel::Warning;

    double currentValue = 0.0;
    double thresholdValue = 0.0;

    QString message;
    bool confirmed = false;

    QDateTime alarmTime = QDateTime::currentDateTime();
    QDateTime confirmedTime;

};

Q_DECLARE_METATYPE(AlarmRecord)

#endif // ALARMRECORD_H
