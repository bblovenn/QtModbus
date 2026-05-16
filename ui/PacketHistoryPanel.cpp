#include "PacketHistoryPanel.h"
#include "../database/DatabaseManager.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QFile>
#include <QFileDialog>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QStringConverter>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextStream>
#include <QVBoxLayout>

PacketHistoryPanel::PacketHistoryPanel(DatabaseManager *databaseManagerValue, QWidget *parent)
    : QWidget(parent)
    , databaseManager(databaseManagerValue)
{
    setupUi();
}

void PacketHistoryPanel::setupUi()
{
    beginTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(-3600), this);
    beginTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    beginTimeEdit->setCalendarPopup(true);

    endTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime(), this);
    endTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    endTimeEdit->setCalendarPopup(true);

    categoryCombo = new QComboBox(this);
    categoryCombo->addItem("全部", "All");
    categoryCombo->addItem("连接", "Connection");
    categoryCombo->addItem("Modbus", "Modbus");
    categoryCombo->addItem("通信", "Communication");
    categoryCombo->addItem("报警", "Alarm");

    queryButton = new QPushButton("查询", this);
    exportButton = new QPushButton("导出 CSV", this);

    auto *filterLayout = new QHBoxLayout;
    filterLayout->addWidget(beginTimeEdit);
    filterLayout->addWidget(endTimeEdit);
    filterLayout->addWidget(categoryCombo);
    filterLayout->addWidget(queryButton);
    filterLayout->addWidget(exportButton);

    table = new QTableWidget(this);
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({
        "时间", "类别", "方向", "内容"
    });
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(filterLayout);
    mainLayout->addWidget(table);

    connect(queryButton, &QPushButton::clicked,
            this, &PacketHistoryPanel::queryHistory);

    connect(exportButton, &QPushButton::clicked,
            this, &PacketHistoryPanel::exportCsv);
}

void PacketHistoryPanel::queryHistory()
{
    // category 为空或 All 时查询全部类别；其他值由 DatabaseManager 做精确过滤。
    if (!databaseManager) {
        return;
    }

    currentLogs = databaseManager->queryPacketLogs(
        beginTimeEdit->dateTime(),
        endTimeEdit->dateTime(),
        categoryCombo->currentData().toString()
    );

    displayLogs(currentLogs);
}

void PacketHistoryPanel::displayLogs(const QList<QStringList> &logs)
{
    table->setRowCount(0);

    for (const QStringList &rowData : logs) {
        const int row = table->rowCount();
        table->insertRow(row);

        for (int column = 0; column < rowData.size() && column < 4; ++column) {
            table->setItem(row, column, new QTableWidgetItem(rowData.at(column)));
        }
    }
}

QString PacketHistoryPanel::csvEscape(const QString &value) const
{
    QString escaped = value;
    escaped.replace("\"", "\"\"");
    return "\"" + escaped + "\"";
}

void PacketHistoryPanel::exportCsv()
{
    // 报文日志导出的是当前查询结果，适合和现场操作记录一起归档。
    if (currentLogs.isEmpty()) {
        QMessageBox::information(this, "导出 CSV", "没有可导出的报文日志。");
        return;
    }

    const QString fileName = QFileDialog::getSaveFileName(
        this,
        "导出报文日志",
        "packet_log.csv",
        "CSV 文件 (*.csv)"
    );

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "导出 CSV", "打开文件失败。");
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out.setGenerateByteOrderMark(true);

    out << csvEscape("时间") << ","
        << csvEscape("类别") << ","
        << csvEscape("方向") << ","
        << csvEscape("内容") << "\n";

    for (const QStringList &rowData : currentLogs) {
        out << csvEscape(rowData.value(0)) << ","
            << csvEscape(rowData.value(1)) << ","
            << csvEscape(rowData.value(2)) << ","
            << csvEscape(rowData.value(3)) << "\n";
    }

    file.close();

    QMessageBox::information(this, "导出 CSV", "导出完成。");
}
