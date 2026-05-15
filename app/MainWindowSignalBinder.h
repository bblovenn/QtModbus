#ifndef MAINWINDOWSIGNALBINDER_H
#define MAINWINDOWSIGNALBINDER_H

#include "MainWindowPanels.h"
#include "MainWindowRuntimeState.h"

class AlarmManager;
class AppSettings;
class DatabaseManager;
class PacketLogService;
class PollingWorker;
class QMainWindow;
class QtModbusClient;
class ReconnectController;

// 绑定信号槽需要的上下文对象。集中传入，避免各个 Binder 直接依赖 MainWindow 私有成员。
struct MainWindowSignalBinderContext
{
    QMainWindow *window = nullptr;
    AppSettings *settings = nullptr;
    MainWindowPanels *panels = nullptr;
    MainWindowRuntimeState *runtime = nullptr;
    PacketLogService *packetLog = nullptr;
    QtModbusClient *modbusClient = nullptr;
    PollingWorker *pollingWorker = nullptr;
    AlarmManager *alarmManager = nullptr;
    DatabaseManager *databaseManager = nullptr;
    ReconnectController *reconnectController = nullptr;
};

// 主窗口信号槽总调度器，按业务域分发到各个专用 Binder。
class MainWindowSignalBinder
{
public:
    static void bind(const MainWindowSignalBinderContext &context);
};

#endif // MAINWINDOWSIGNALBINDER_H
