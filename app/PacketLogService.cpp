#include "PacketLogService.h"

#include "../database/DatabaseManager.h"
#include "../ui/PacketMonitorPanel.h"

#include <QDateTime>

PacketLogService::PacketLogService(PacketMonitorPanel *panel, DatabaseManager *database)
    : panel(panel)
    , database(database)
{
}

void PacketLogService::append(const QString &category,
                              const QString &direction,
                              const QString &content)
{
    // UI 显示和数据库保存使用同一时间戳，保证两边日志可以对齐。
    const QDateTime now = QDateTime::currentDateTime();

    panel->appendText(
        QString("[%1] %2: %3")
            .arg(now.toString("yyyy-MM-dd HH:mm:ss.zzz"))
            .arg(direction)
            .arg(content)
    );

    database->savePacketLog(
        now,
        category,
        direction,
        content
    );
}
