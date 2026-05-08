#ifndef ALARMMANAGER_H
#define ALARMMANAGER_H

#include "../core/AlarmRecord.h"
#include "../core/EngineeringValue.h"

#include <QObject>

class AlarmManager : public QObject
{
    Q_OBJECT

public:
    explicit AlarmManager(QObject *parent = nullptr);

public slots:
    void checkValue(const EngineeringValue &value);
    void onDeviceOffline();
    void onDeviceOfflineForDevice(const QString &deviceId);

signals:
    void alarmRaised(const AlarmRecord &alarm);

private:
    AlarmRecord createAlarm(
        const QString &deviceId,
        AlarmType type,
        AlarmLevel level,
        double currentValue,
        double thresholdValue,
        const QString &message
    ) const;

private:
    bool temperatureHighActive = false;
    bool voltageLowActive = false;
    bool deviceOfflineActive = false;

    double temperatureHighLimit = 80.0;
    double voltageLowLimit = 18.0;
};

#endif // ALARMMANAGER_H
