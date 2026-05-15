#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include "../core/AlarmRecord.h"
#include "../core/EngineeringValue.h"

#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include <QDateTime>
#include <QStringList>

// SQLite 数据访问入口。负责建表、写入采集/报警/报文日志，并为历史页面提供查询接口。
class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager(); 

    bool open(const QString &databasePath);
    bool initialize(); // 初始化表结构（建表）
    QString databaseFilePath() const; 
    int tableRowCount(const QString &tableName); // 返回指定表的行数
    bool deleteDataBefore(const QDateTime &cutoffTime);  //删除 cutoffTime 之前的所有记录
    bool vacuum();//回收已删除数据占用的磁盘空间

    QList<EngineeringValue> queryEngineeringValues(
    const QString &deviceId,
    const QDateTime &beginTime,
    const QDateTime &endTime
    );

    QList<AlarmRecord> queryAlarmRecords(
    const QString &deviceId,
    const QDateTime &beginTime,
    const QDateTime &endTime,
    int confirmedFilter
    );

    QList<QStringList> queryPacketLogs(
    const QDateTime &beginTime,
    const QDateTime &endTime,
    const QString &category
    );

public slots:
    // 这些槽直接接收业务层信号，写库失败会统一通过 errorOccurred 报出。
    void saveEngineeringValue(const EngineeringValue &value);
    void saveAlarmRecord(const AlarmRecord &alarm);
    void confirmAlarm(const QString &alarmId, const QDateTime &confirmedTime);
    void savePacketLog(
    const QDateTime &time,
    const QString &category,
    const QString &direction,
    const QString &content
    );

signals:
    void errorOccurred(const QString &message);

private:
    // 数据库中保存中文枚举文本，查询时需要在文本和枚举之间做双向转换。
    QString alarmTypeText(AlarmType type) const;
    QString alarmLevelText(AlarmLevel level) const;
    //反向转换函数，将数据库中的文本转回枚举类型
    AlarmType alarmTypeFromText(const QString &text) const;
    AlarmLevel alarmLevelFromText(const QString &text) const;

private:
    QSqlDatabase database;
    QString connectionName;
};

#endif // DATABASEMANAGER_H
