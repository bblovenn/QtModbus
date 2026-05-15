#ifndef ALARMRECORD_H
#define ALARMRECORD_H

#include <QDateTime>
#include <QMetaType>
#include <QString>

// 报警等级，用于决定 UI 提示强度和历史查询分类。
enum class AlarmLevel{
    Info,
    Warning,
    Critical
};

// 报警类型，覆盖采集值越限、通信异常和 Modbus 异常等场景。
enum class AlarmType{
    TemperatureHigh,
    VoltageLow,
    CommunicationTimeout,
    DeviceOffline,
    ModbusException
};

// 一条报警记录。实时报警面板和 SQLite 历史表共用这份数据结构。
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
