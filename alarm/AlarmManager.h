#ifndef ALARMMANAGER_H
#define ALARMMANAGER_H

#include "../core/AlarmRecord.h"
#include "../core/EngineeringValue.h"

#include <QObject>

// 报警管理器：根据工程量和通信状态生成报警，并抑制同一报警的重复触发。
class AlarmManager : public QObject
{
    Q_OBJECT

public:
    explicit AlarmManager(QObject *parent = nullptr);

    double temperatureHighLimitValue() const;
    double voltageLowLimitValue() const;

public slots:
    void checkValue(const EngineeringValue &value);
    void onDeviceOffline();
    void onDeviceOfflineForDevice(const QString &deviceId);
    void setAlarmLimits(double temperatureHighLimit, double voltageLowLimit);

signals:
    void alarmRaised(const AlarmRecord &alarm);

private:
    // 创建报警记录时统一补齐 ID、时间、设备、等级和消息，方便后续持久化。
    AlarmRecord createAlarm(
        const QString &deviceId,
        AlarmType type,
        AlarmLevel level,
        double currentValue,
        double thresholdValue,
        const QString &message
    ) const;

private:
    // Active 标志用于避免同一个越限条件在每次轮询时都重复产生报警。
    bool temperatureHighActive = false;
    bool voltageLowActive = false;
    bool deviceOfflineActive = false;

    double temperatureHighLimit = 80.0;
    double voltageLowLimit = 18.0;
};

#endif // ALARMMANAGER_H
