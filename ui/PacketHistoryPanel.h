#ifndef PACKETHISTORYPANEL_H
#define PACKETHISTORYPANEL_H

#include <QList>
#include <QStringList>
#include <QWidget>

class DatabaseManager;
class QComboBox;
class QDateTimeEdit;
class QPushButton;
class QTableWidget;

// 报文历史面板：查询数据库中的报文日志，并支持按类别/方向筛选。
class PacketHistoryPanel : public QWidget
{
    Q_OBJECT

public:
    explicit PacketHistoryPanel(DatabaseManager *databaseManager, QWidget *parent = nullptr);

private slots:
    void queryHistory();
    void exportCsv();

private:
    void setupUi();
    void displayLogs(const QList<QStringList> &logs);
    //配合 exportCsv() 槽函数使用，在导出为 CSV 格式时对字段值进行转义
    QString csvEscape(const QString &value) const;

private:
    DatabaseManager *databaseManager = nullptr;

    QDateTimeEdit *beginTimeEdit = nullptr;
    QDateTimeEdit *endTimeEdit = nullptr;
    QComboBox *categoryCombo = nullptr;
    QPushButton *queryButton = nullptr;
    QPushButton *exportButton = nullptr;
    QTableWidget *table = nullptr;

    QList<QStringList> currentLogs;
};

#endif // PACKETHISTORYPANEL_H
