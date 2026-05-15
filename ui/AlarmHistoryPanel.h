#ifndef ALARMHISTORYPANEL_H
#define ALARMHISTORYPANEL_H

#include "../core/AlarmRecord.h"

#include <QWidget>
#include <QList>

class DatabaseManager;
class QComboBox;
class QDateTimeEdit;
class QLineEdit;
class QPushButton;
class QTableWidget;

// 报警历史面板：按条件查询报警记录，并支持导出 CSV。
class AlarmHistoryPanel : public QWidget
{
    Q_OBJECT

public:
    explicit AlarmHistoryPanel(DatabaseManager *databaseManager, QWidget *parent = nullptr);

private slots:
    void queryHistory();
    void exportCsv();

private:
    void setupUi();
    void displayRecords(const QList<AlarmRecord> &records);
    QString alarmTypeText(AlarmType type) const;
    QString alarmLevelText(AlarmLevel level) const;

private:
    DatabaseManager *databaseManager = nullptr;

    QLineEdit *deviceIdEdit = nullptr;
    QDateTimeEdit *beginTimeEdit = nullptr;
    QDateTimeEdit *endTimeEdit = nullptr;
    QComboBox *confirmedCombo = nullptr;
    QPushButton *queryButton = nullptr;
    QTableWidget *table = nullptr;
    QPushButton *exportButton = nullptr;
    QList<AlarmRecord> currentRecords;
};

#endif // ALARMHISTORYPANEL_H
