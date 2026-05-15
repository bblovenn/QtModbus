#ifndef DATABASEMAINTENANCEPANEL_H
#define DATABASEMAINTENANCEPANEL_H

#include <QWidget>

class DatabaseManager;
class QLabel;
class QDateTimeEdit;
class QPushButton;

// 数据库维护面板：展示数据库状态，并提供清理历史数据和压缩数据库入口。
class DatabaseMaintenancePanel : public QWidget
{
    Q_OBJECT

public:
    explicit DatabaseMaintenancePanel(DatabaseManager *databaseManager, QWidget *parent = nullptr);

private slots:
    void cleanupOldData();
    void vacuumDatabase();

public slots:
    void refreshInfo();

private:
    void setupUi();

private:
    DatabaseManager *databaseManager = nullptr;

    QLabel *pathLabel = nullptr;
    QLabel *collectCountLabel = nullptr;
    QLabel *alarmCountLabel = nullptr;
    QLabel *packetCountLabel = nullptr;

    QDateTimeEdit *cutoffTimeEdit = nullptr;
    QPushButton *refreshButton = nullptr;
    QPushButton *cleanupButton = nullptr;
    QPushButton *vacuumButton = nullptr;
};

#endif // DATABASEMAINTENANCEPANEL_H
