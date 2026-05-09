#include "DatabaseManager.h"

#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery> // SQL 查询执行器
#include <QUuid>
#include <QVariant>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
    // SQL 查询执行器
    connectionName = "modbus_hmi_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
}

DatabaseManager::~DatabaseManager()
{
    if (database.isOpen()) {
        database.close();
    }
    database = QSqlDatabase();
    QSqlDatabase::removeDatabase(connectionName);
}

bool DatabaseManager::open(const QString &databasePath)
{
    // 添加 SQLite 驱动，创建数据库实例
    database = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    database.setDatabaseName(databasePath);

    if (!database.open()) {
        emit errorOccurred(database.lastError().text());
        return false;
    }

    return true;
}

bool DatabaseManager::initialize()
{
    // SQL 查询执行器
    QSqlQuery query(database);

    const QString collectSql =
        "CREATE TABLE IF NOT EXISTS collect_data ("  
        "id INTEGER PRIMARY KEY AUTOINCREMENT," // 自增主键
        "device_id TEXT NOT NULL," // 设备ID
        "collect_time TEXT NOT NULL," // 采集时间
        "temperature REAL,"
        "voltage REAL,"
        "current REAL,"
        "speed REAL"
        ")";

    // 执行建表 SQL
    if (!query.exec(collectSql)) {
        emit errorOccurred(query.lastError().text());
        return false;
    }

    // 定义建表 SQL：存储报警日志
    const QString alarmSql =
        "CREATE TABLE IF NOT EXISTS alarm_log ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT," // 自增主键
        "alarm_id TEXT NOT NULL," // 报警唯一ID
        "alarm_time TEXT NOT NULL,"
        "device_id TEXT NOT NULL,"
        "alarm_type TEXT NOT NULL,"
        "alarm_level TEXT NOT NULL,"
        "current_value REAL,"
        "threshold_value REAL,"
        "message TEXT,"
        "confirmed INTEGER DEFAULT 0,"
        "confirmed_time TEXT"
        ")";

    if (!query.exec(alarmSql)) {
        emit errorOccurred(query.lastError().text());
        return false;
    }

    return true;
}

// 保存工程值到数据库
void DatabaseManager::saveEngineeringValue(const EngineeringValue &value)
{
    if (!database.isOpen()) {
        emit errorOccurred("数据库未打开");
        return;
    }

    QSqlQuery query(database);
    query.prepare(
        "INSERT INTO collect_data "
        "(device_id, collect_time, temperature, voltage, current, speed) "
        "VALUES "
        "(:device_id, :collect_time, :temperature, :voltage, :current, :speed)"
    );

    query.bindValue(":device_id", value.deviceId);
    query.bindValue(":collect_time", value.timestamp.toString(Qt::ISODate)); // 时间转 ISO 格式
    query.bindValue(":temperature", value.temperature);
    query.bindValue(":voltage", value.voltage);
    query.bindValue(":current", value.current);
    query.bindValue(":speed", value.speed);

    if (!query.exec()) {
        emit errorOccurred(query.lastError().text());
    }
}

// 保存报警记录到数据库
void DatabaseManager::saveAlarmRecord(const AlarmRecord &alarm)
{
    if (!database.isOpen()) {
        emit errorOccurred("数据库未打开");
        return;
    }

    // 预编译 INSERT 语句
    QSqlQuery query(database);
    query.prepare(
        "INSERT INTO alarm_log "
        "(alarm_id, alarm_time, device_id, alarm_type, alarm_level, "
        "current_value, threshold_value, message, confirmed, confirmed_time) "
        "VALUES "
        "(:alarm_id, :alarm_time, :device_id, :alarm_type, :alarm_level, "
        ":current_value, :threshold_value, :message, :confirmed, :confirmed_time)"
    );

    query.bindValue(":alarm_id", alarm.id);
    query.bindValue(":alarm_time", alarm.alarmTime.toString(Qt::ISODate));
    query.bindValue(":device_id", alarm.deviceId);
    query.bindValue(":alarm_type", alarmTypeText(alarm.type));   // 枚举转中文
    query.bindValue(":alarm_level", alarmLevelText(alarm.level)); // 枚举转中文
    query.bindValue(":current_value", alarm.currentValue);
    query.bindValue(":threshold_value", alarm.thresholdValue);
    query.bindValue(":message", alarm.message);
    query.bindValue(":confirmed", alarm.confirmed ? 1 : 0);
    // 确认时间：如果有效则转 ISO 格式，否则存空字符串
    query.bindValue(":confirmed_time", alarm.confirmedTime.isValid()
        ? alarm.confirmedTime.toString(Qt::ISODate)
        : QString()
    );

    if (!query.exec()) {
        emit errorOccurred(query.lastError().text());
    }
}

// 报警类型枚举转中文字符串，数据库中也保存中文。
QString DatabaseManager::alarmTypeText(AlarmType type) const
{
    switch (type) {
    case AlarmType::TemperatureHigh:
        return "温度过高";
    case AlarmType::VoltageLow:
        return "电压过低";
    case AlarmType::DeviceOffline:
        return "设备离线";
    case AlarmType::CommunicationTimeout:
        return "通信超时";
    case AlarmType::ModbusException:
        return "Modbus异常";
    }

    return "未知";
}

// 报警等级枚举转中文字符串，数据库中也保存中文。
QString DatabaseManager::alarmLevelText(AlarmLevel level) const
{
    switch (level) {
    case AlarmLevel::Info:
        return "提示";
    case AlarmLevel::Warning:
        return "警告";
    case AlarmLevel::Critical:
        return "严重";
    }

    return "未知";
}

AlarmType DatabaseManager::alarmTypeFromText(const QString &text) const
{
    if (text == "温度过高" || text == "TemperatureHigh") return AlarmType::TemperatureHigh;
    if (text == "电压过低" || text == "VoltageLow") return AlarmType::VoltageLow;
    if (text == "设备离线" || text == "DeviceOffline") return AlarmType::DeviceOffline;
    if (text == "通信超时" || text == "CommunicationTimeout") return AlarmType::CommunicationTimeout;
    if (text == "Modbus异常" || text == "ModbusException") return AlarmType::ModbusException;
    return AlarmType::TemperatureHigh;
}

AlarmLevel DatabaseManager::alarmLevelFromText(const QString &text) const
{
    if (text == "提示" || text == "Info") return AlarmLevel::Info;
    if (text == "警告" || text == "Warning") return AlarmLevel::Warning;
    if (text == "严重" || text == "Critical") return AlarmLevel::Critical;
    return AlarmLevel::Warning;
}


// 查询指定设备和时间范围内的工程值列表
QList<EngineeringValue> DatabaseManager::queryEngineeringValues(
    const QString &deviceId,
    const QDateTime &beginTime,
    const QDateTime &endTime
)
{
    QList<EngineeringValue> values;

    if (!database.isOpen()) {
        emit errorOccurred("database is not open");
        return values;
    }

    QSqlQuery query(database);
    query.prepare(
        "SELECT device_id, collect_time, temperature, voltage, current, speed "
        "FROM collect_data "
        "WHERE device_id = :device_id "
        "AND collect_time >= :begin_time "
        "AND collect_time <= :end_time "
        "ORDER BY collect_time ASC"
    );

    query.bindValue(":device_id", deviceId);
    query.bindValue(":begin_time", beginTime.toString(Qt::ISODate));
    query.bindValue(":end_time", endTime.toString(Qt::ISODate));

    if (!query.exec()) {
        emit errorOccurred(query.lastError().text());
        return values;
    }

    while (query.next()) {
        EngineeringValue value;
        value.deviceId = query.value("device_id").toString();
        value.timestamp = QDateTime::fromString(
            query.value("collect_time").toString(),
            Qt::ISODate
        );
        value.temperature = query.value("temperature").toDouble();
        value.voltage = query.value("voltage").toDouble();
        value.current = query.value("current").toDouble();
        value.speed = query.value("speed").toDouble();

        values.append(value);
    }

    return values;
}

void DatabaseManager::confirmAlarm(const QString &alarmId, const QDateTime &confirmedTime)
{
    if (!database.isOpen()) {
        emit errorOccurred("database is not open");
        return;
    }

    QSqlQuery query(database);
    query.prepare(
        "UPDATE alarm_log "
        "SET confirmed = 1, "
        "confirmed_time = :confirmed_time "
        "WHERE alarm_id = :alarm_id"
    );

    query.bindValue(":confirmed_time", confirmedTime.toString(Qt::ISODate));
    query.bindValue(":alarm_id", alarmId);

    if (!query.exec()) {
        emit errorOccurred(query.lastError().text());
        return;
    }

    if (query.numRowsAffected() <= 0) {
        emit errorOccurred("alarm record not found: " + alarmId);
    }
}

QList<AlarmRecord> DatabaseManager::queryAlarmRecords(
    const QString &deviceId,
    const QDateTime &beginTime,
    const QDateTime &endTime,
    int confirmedFilter
)
{
    QList<AlarmRecord> records;

    if (!database.isOpen()) {
        emit errorOccurred("database is not open");
        return records;
    }

    QString sql =
        "SELECT alarm_id, alarm_time, device_id, alarm_type, alarm_level, "
        "current_value, threshold_value, message, confirmed, confirmed_time "
        "FROM alarm_log "
        "WHERE alarm_time >= :begin_time "
        "AND alarm_time <= :end_time ";

    if (!deviceId.trimmed().isEmpty()) {
        sql += "AND device_id = :device_id ";
    }

    //只有当过滤条件是 0 或 1 时才添加此条件
    if (confirmedFilter == 0 || confirmedFilter == 1) {
        sql += "AND confirmed = :confirmed ";
    }

    //按报警时间降序排列（最新的记录排在最前面）
    sql += "ORDER BY alarm_time DESC";

    QSqlQuery query(database);
    query.prepare(sql);

    query.bindValue(":begin_time", beginTime.toString(Qt::ISODate));
    query.bindValue(":end_time", endTime.toString(Qt::ISODate));

    if (!deviceId.trimmed().isEmpty()) {
        query.bindValue(":device_id", deviceId.trimmed());
    }

    //绑定确认状态过滤参数
    if (confirmedFilter == 0 || confirmedFilter == 1) {
        query.bindValue(":confirmed", confirmedFilter);
    }

    if (!query.exec()) {
        emit errorOccurred(query.lastError().text());
        return records;
    }

    while (query.next()) {
        AlarmRecord record;
        record.id = query.value("alarm_id").toString();
        record.alarmTime = QDateTime::fromString(query.value("alarm_time").toString(), Qt::ISODate);
        record.deviceId = query.value("device_id").toString();
        record.type = alarmTypeFromText(query.value("alarm_type").toString());
        record.level = alarmLevelFromText(query.value("alarm_level").toString());
        record.currentValue = query.value("current_value").toDouble();
        record.thresholdValue = query.value("threshold_value").toDouble();
        record.message = query.value("message").toString();
        record.confirmed = query.value("confirmed").toInt() != 0;

        const QString confirmedTimeText = query.value("confirmed_time").toString();
        if (!confirmedTimeText.isEmpty()) {
            record.confirmedTime = QDateTime::fromString(confirmedTimeText, Qt::ISODate);
        }

        records.append(record);
    }

    return records;
}


