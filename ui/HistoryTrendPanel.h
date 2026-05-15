#ifndef HISTORYTRENDPANEL_H
#define HISTORYTRENDPANEL_H

#include <QWidget>

class DatabaseManager;
class QDateTimeEdit;
class QLineEdit;
class QPushButton;
class TrendPanel;

// 历史曲线面板：从数据库查询历史采集值，并复用趋势图进行展示。
class HistoryTrendPanel : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryTrendPanel(DatabaseManager *databaseManager, QWidget *parent = nullptr);

private slots:
    void queryTrend();
    void clearTrend();

private:
    void setupUi();

private:
    DatabaseManager *databaseManager = nullptr;

    QLineEdit *deviceIdEdit = nullptr;
    QDateTimeEdit *beginTimeEdit = nullptr;
    QDateTimeEdit *endTimeEdit = nullptr;
    QPushButton *queryButton = nullptr;
    QPushButton *clearButton = nullptr;
    TrendPanel *trendPanel = nullptr;
};

#endif
