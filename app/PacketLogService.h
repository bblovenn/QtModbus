#ifndef PACKETLOGSERVICE_H
#define PACKETLOGSERVICE_H

#include <QString>

class DatabaseManager;
class PacketMonitorPanel;

// 报文日志服务：同时写入 UI 文本框和数据库，保证显示与持久化路径一致。
class PacketLogService
{
public:
    PacketLogService(PacketMonitorPanel *panel, DatabaseManager *database);

    void append(const QString &category,
                const QString &direction,
                const QString &content);

private:
    PacketMonitorPanel *panel = nullptr;
    DatabaseManager *database = nullptr;
};

#endif // PACKETLOGSERVICE_H
