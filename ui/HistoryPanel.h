#ifndef HISTORYPANEL_H
#define HISTORYPANEL_H

#include "../core/EngineeringValue.h"

#include <QWidget>

class DatabaseManager;
class QDateTimeEdit;
class QLineEdit;
class QPushButton;
class QTableWidget;

class HistoryPanel : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryPanel(DatabaseManager *databaseManager, QWidget *parent = nullptr);

private slots:
    void queryHistory();

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
};

#endif // HISTORYPANEL_H
