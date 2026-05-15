#ifndef HISTORYPANEL_H
#define HISTORYPANEL_H

#include "../core/EngineeringValue.h"

#include <QWidget>
#include <QList>

class DatabaseManager;
class QDateTimeEdit;
class QLineEdit;
class QPushButton;
class QTableWidget;

// 采集历史面板：查询工程量采集数据，并支持导出 CSV。
class HistoryPanel : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryPanel(DatabaseManager *databaseManager, QWidget *parent = nullptr);

private slots:
    void queryHistory();
    void exportCsv();

private:
    void setupUi();
    void displayValues(const QList<EngineeringValue> &values);

private:
    DatabaseManager *databaseManager = nullptr; //数据库操作指针

    QLineEdit *deviceIdEdit = nullptr;
    QDateTimeEdit *beginTimeEdit = nullptr;
    QDateTimeEdit *endTimeEdit = nullptr;
    QPushButton *queryButton = nullptr;
    QTableWidget *table = nullptr;
    QPushButton *exportButton = nullptr;
    QList<EngineeringValue> currentValues; // 当前查询结果，用于导出
};

#endif // HISTORYPANEL_H
