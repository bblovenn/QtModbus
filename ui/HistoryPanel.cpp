#include "HistoryPanel.h"
#include "../database/DatabaseManager.h"

#include <QAbstractItemView>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QFile>
#include <QFileDialog>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextStream>
#include <QVBoxLayout>

namespace {

QString text(const char *value)
{
    return QString::fromUtf8(value);
}

QString csvEscape(const QString &value)
{
    QString escaped = value;
    escaped.replace("\"", "\"\"");
    return "\"" + escaped + "\"";
}

} // namespace

HistoryPanel::HistoryPanel(DatabaseManager *databaseManagerValue, QWidget *parent)
    : QWidget(parent)
    , databaseManager(databaseManagerValue)
{
    setupUi();
}

void HistoryPanel::setupUi()
{
    deviceIdEdit = new QLineEdit("device-001", this);

    beginTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(-3600), this);
    beginTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    beginTimeEdit->setCalendarPopup(true);

    endTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime(), this);
    endTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    endTimeEdit->setCalendarPopup(true);

    queryButton = new QPushButton(text(u8"查询"), this);
    exportButton = new QPushButton(text(u8"导出 CSV"), this);

    auto *filterLayout = new QHBoxLayout;
    filterLayout->addWidget(deviceIdEdit);
    filterLayout->addWidget(beginTimeEdit);
    filterLayout->addWidget(endTimeEdit);
    filterLayout->addWidget(queryButton);
    filterLayout->addWidget(exportButton);

    table = new QTableWidget(this);
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({
        text(u8"时间"),
        text(u8"设备 ID"),
        text(u8"温度"),
        text(u8"电压"),
        text(u8"电流"),
        text(u8"速度")
    });
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(filterLayout);
    mainLayout->addWidget(table);

    connect(queryButton, &QPushButton::clicked,
            this, &HistoryPanel::queryHistory);

    connect(exportButton, &QPushButton::clicked,
            this, &HistoryPanel::exportCsv);
}

void HistoryPanel::queryHistory()
{
    if (!databaseManager) {
        return;
    }

    currentValues = databaseManager->queryEngineeringValues(
        deviceIdEdit->text().trimmed(),
        beginTimeEdit->dateTime(),
        endTimeEdit->dateTime()
    );

    displayValues(currentValues);
}

void HistoryPanel::displayValues(const QList<EngineeringValue> &values)
{
    table->setRowCount(0);

    for (const EngineeringValue &value : values) {
        const int row = table->rowCount();
        table->insertRow(row);

        table->setItem(row, 0, new QTableWidgetItem(value.timestamp.toString("yyyy-MM-dd HH:mm:ss")));
        table->setItem(row, 1, new QTableWidgetItem(value.deviceId));
        table->setItem(row, 2, new QTableWidgetItem(QString::number(value.temperature, 'f', 2)));
        table->setItem(row, 3, new QTableWidgetItem(QString::number(value.voltage, 'f', 2)));
        table->setItem(row, 4, new QTableWidgetItem(QString::number(value.current, 'f', 2)));
        table->setItem(row, 5, new QTableWidgetItem(QString::number(value.speed, 'f', 0)));
    }
}

void HistoryPanel::exportCsv()
{
    if (currentValues.isEmpty()) {
        QMessageBox::information(this, text(u8"导出 CSV"), text(u8"没有数据可以导出。"));
        return;
    }

    const QString fileName = QFileDialog::getSaveFileName(
        this,
        text(u8"导出采集数据"),
        "collect_data.csv",
        text(u8"CSV 文件 (*.csv)")
    );

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, text(u8"导出 CSV"), text(u8"无法打开文件。"));
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out.setGenerateByteOrderMark(true);

    out << csvEscape(text(u8"时间")) << ","
        << csvEscape(text(u8"设备 ID")) << ","
        << csvEscape(text(u8"温度")) << ","
        << csvEscape(text(u8"电压")) << ","
        << csvEscape(text(u8"电流")) << ","
        << csvEscape(text(u8"速度")) << "\n";

    for (const EngineeringValue &value : currentValues) {
        out << csvEscape(value.timestamp.toString("yyyy-MM-dd HH:mm:ss")) << ","
            << csvEscape(value.deviceId) << ","
            << csvEscape(QString::number(value.temperature, 'f', 2)) << ","
            << csvEscape(QString::number(value.voltage, 'f', 2)) << ","
            << csvEscape(QString::number(value.current, 'f', 2)) << ","
            << csvEscape(QString::number(value.speed, 'f', 0)) << "\n";
    }

    file.close();

    QMessageBox::information(this, text(u8"导出 CSV"), text(u8"导出完成。"));
}
