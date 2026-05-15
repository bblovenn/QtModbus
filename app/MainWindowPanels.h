#ifndef MAINWINDOWPANELS_H
#define MAINWINDOWPANELS_H

class AlarmConfigPanel;
class AlarmHistoryPanel;
class AlarmPanel;
class ConnectionPanel;
class DatabaseMaintenancePanel;
class HistoryPanel;
class HistoryTrendPanel;
class MonitorPanel;
class PacketHistoryPanel;
class PacketMonitorPanel;
class PollingConfigPanel;
class QTabWidget;
class ReconnectConfigPanel;
class RegisterPanel;
class SystemStatusPanel;
class TrendPanel;

// MainWindow 创建的所有面板指针集合。对象所有权仍交给 Qt parent-child 机制管理。
struct MainWindowPanels
{
    QTabWidget *mainTabs = nullptr;
    QTabWidget *historyTabs = nullptr;

    ConnectionPanel *connection = nullptr;
    RegisterPanel *registers = nullptr;
    MonitorPanel *monitor = nullptr;
    AlarmPanel *alarms = nullptr;
    PacketMonitorPanel *packets = nullptr;
    TrendPanel *trend = nullptr;
    SystemStatusPanel *systemStatus = nullptr;
    ReconnectConfigPanel *reconnectConfig = nullptr;
    HistoryPanel *history = nullptr;
    AlarmHistoryPanel *alarmHistory = nullptr;
    HistoryTrendPanel *historyTrend = nullptr;
    PacketHistoryPanel *packetHistory = nullptr;
    DatabaseMaintenancePanel *databaseMaintenance = nullptr;
    PollingConfigPanel *pollingConfig = nullptr;
    AlarmConfigPanel *alarmConfig = nullptr;
};

#endif // MAINWINDOWPANELS_H
