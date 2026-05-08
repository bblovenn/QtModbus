#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include "../core/AlarmRecord.h"
#include "../core/EngineeringValue.h"

#include <QObject>
#include <QSqlDatabase>
#include <QList>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager(); 

    bool open(const QString &databasePath);
    bool initialize(); // 初始化表结构（建表）

    QList<EngineeringValue> queryEngineeringValues(
    const QString &deviceId,
    const QDateTime &beginTime,
    const QDateTime &endTime
);

public slots:
    void saveEngineeringValue(const EngineeringValue &value);
    void saveAlarmRecord(const AlarmRecord &alarm);

signals:
    void errorOccurred(const QString &message);

private:
    QString alarmTypeText(AlarmType type) const;
    QString alarmLevelText(AlarmLevel level) const;

private:
    QSqlDatabase database;
    QString connectionName;
};

#endif // DATABASEMANAGER_H
