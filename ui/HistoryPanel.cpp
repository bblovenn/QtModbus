#include "HistoryPanel.h"
#include "../database/DatabaseManager.h"

#include <QDateTime>
#include <QDateTimeEdit>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

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

    queryButton = new QPushButton("Query", this);

    auto *filterLayout = new QHBoxLayout;
    filterLayout->addWidget(deviceIdEdit);
    filterLayout->addWidget(beginTimeEdit);
    filterLayout->addWidget(endTimeEdit);
    filterLayout->addWidget(queryButton);

    table = new QTableWidget(this);
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({
        "Time", "Device ID", "Temperature", "Voltage", "Current", "Speed"
    });
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(filterLayout);
    mainLayout->addWidget(table);

    connect(queryButton, &QPushButton::clicked,
            this, &HistoryPanel::queryHistory);
}

void HistoryPanel::queryHistory()
{
    if (!databaseManager) {
        return;
    }

    const QList<EngineeringValue> values =
        databaseManager->queryEngineeringValues(
            deviceIdEdit->text().trimmed(),
            beginTimeEdit->dateTime(),
            endTimeEdit->dateTime()
        );

    displayValues(values);
}

void HistoryPanel::displayValues(const QList<EngineeringValue> &values)
{
    table->setRowCount(0);

    for (const EngineeringValue &value : values) {
        const int row = table->rowCount();
        table->insertRow(row);

        table->setItem(row, 0, new QTableWidgetItem(
            value.timestamp.toString("yyyy-MM-dd HH:mm:ss")
        ));
        table->setItem(row, 1, new QTableWidgetItem(value.deviceId));
        table->setItem(row, 2, new QTableWidgetItem(QString::number(value.temperature, 'f', 2)));
        table->setItem(row, 3, new QTableWidgetItem(QString::number(value.voltage, 'f', 2)));
        table->setItem(row, 4, new QTableWidgetItem(QString::number(value.current, 'f', 2)));
        table->setItem(row, 5, new QTableWidgetItem(QString::number(value.speed, 'f', 0)));
    }
}
