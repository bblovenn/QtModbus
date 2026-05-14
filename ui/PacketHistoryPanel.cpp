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
    categoryCombo->addItem("All", "All");
    categoryCombo->addItem("Connection", "Connection");
    categoryCombo->addItem("Modbus", "Modbus");
    categoryCombo->addItem("Communication", "Communication");
    categoryCombo->addItem("Alarm", "Alarm");

    queryButton = new QPushButton("Query", this);
    exportButton = new QPushButton("Export CSV", this);

    auto *filterLayout = new QHBoxLayout;
    filterLayout->addWidget(beginTimeEdit);
    filterLayout->addWidget(endTimeEdit);
    filterLayout->addWidget(categoryCombo);
    filterLayout->addWidget(queryButton);
    filterLayout->addWidget(exportButton);

    table = new QTableWidget(this);
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({
        "Time", "Category", "Direction", "Content"
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
    if (currentLogs.isEmpty()) {
        QMessageBox::information(this, "Export CSV", "No packet logs to export.");
        return;
    }

    const QString fileName = QFileDialog::getSaveFileName(
        this,
        "Export Packet Logs",
        "packet_log.csv",
        "CSV Files (*.csv)"
    );

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Export CSV", "Failed to open file.");
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out.setGenerateByteOrderMark(true);

    out << csvEscape("Time") << ","
        << csvEscape("Category") << ","
        << csvEscape("Direction") << ","
        << csvEscape("Content") << "\n";

    for (const QStringList &rowData : currentLogs) {
        out << csvEscape(rowData.value(0)) << ","
            << csvEscape(rowData.value(1)) << ","
            << csvEscape(rowData.value(2)) << ","
            << csvEscape(rowData.value(3)) << "\n";
    }

    file.close();

    QMessageBox::information(this, "Export CSV", "Export completed.");
}
