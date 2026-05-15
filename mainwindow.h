#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "app/AppSettings.h"
#include "app/MainWindowPanels.h"
#include "app/MainWindowRuntimeState.h"

#include <QMainWindow>

#include <memory>

class AlarmManager;
class DatabaseManager;
class PacketLogService;
class PollingWorker;
class QtModbusClient;
class ReconnectController;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// 主窗口只负责创建服务对象、创建页面、加载初始配置，并调用 Binder 完成信号槽装配。
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void setupMainTabs();
    void createServices();
    void createPanels();
    void loadInitialSettings();
    void addTabs();
    void initializeDatabase();
    void connectSignals();

private:
    Ui::MainWindow *ui;
    AppSettings appSettings;
    MainWindowPanels panels;
    MainWindowRuntimeState runtimeState;
    std::unique_ptr<PacketLogService> packetLogService;

    QtModbusClient *modbusClient = nullptr;
    PollingWorker *pollingWorker = nullptr;
    AlarmManager *alarmManager = nullptr;
    DatabaseManager *databaseManager = nullptr;
    ReconnectController *reconnectController = nullptr;
};

#endif // MAINWINDOW_H
