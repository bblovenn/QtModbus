#include "MainWindowSignalBinder.h"

#include "DeviceCommandSignalBinder.h"
#include "PacketLogSignalBinder.h"
#include "PollingAlarmSignalBinder.h"
#include "ReconnectSignalBinder.h"
#include "SystemStatusSignalBinder.h"
#include "../database/DatabaseManager.h"
#include "../ui/PacketMonitorPanel.h"

#include <QDateTime>

namespace {

void bindDatabaseErrors(const MainWindowSignalBinderContext &context)
{
    QObject::connect(context.databaseManager, &DatabaseManager::errorOccurred,
        context.panels->packets, [panels = context.panels](const QString &message) {
            panels->packets->appendText(
                QString("[%1] DB: Error %2")
                    .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"))
                    .arg(message)
            );
    });
}

} // namespace

void MainWindowSignalBinder::bind(const MainWindowSignalBinderContext &context)
{
    // 这里保持为总入口，具体连接细节按业务域拆到专用 Binder 中。
    bindDatabaseErrors(context);
    DeviceCommandSignalBinder::bind(context);
    PollingAlarmSignalBinder::bind(context);
    PacketLogSignalBinder::bind(context);
    SystemStatusSignalBinder::bind(context);
    ReconnectSignalBinder::bind(context);
}
