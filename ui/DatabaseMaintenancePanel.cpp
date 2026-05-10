#include "DatabaseMaintenancePanel.h"
#include "../database/DatabaseManager.h"

#include <QDateTime>
#include <QDateTimeEdit>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

DatabaseMaintenancePanel::DatabaseMaintenancePanel(DatabaseManager *databaseManagerValue, QWidget *parent)
    : QWidget(parent)
    , databaseManager(databaseManagerValue)
{
    setupUi();
}

void DatabaseMaintenancePanel::setupUi()
{
    pathLabel = new QLabel(this);
    collectCountLabel = new QLabel(this);
    alarmCountLabel = new QLabel(this);
    packetCountLabel = new QLabel(this);

    cutoffTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime().addDays(-7), this);
    cutoffTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    cutoffTimeEdit->setCalendarPopup(true);

    refreshButton = new QPushButton("Refresh", this);
    cleanupButton = new QPushButton("Delete Before Time", this);
    vacuumButton = new QPushButton("Vacuum", this);

    auto *formLayout = new QFormLayout;
    formLayout->addRow("Database Path", pathLabel);
    formLayout->addRow("Collect Rows", collectCountLabel);
    formLayout->addRow("Alarm Rows", alarmCountLabel);
    formLayout->addRow("Packet Rows", packetCountLabel);
    formLayout->addRow("Cutoff Time", cutoffTimeEdit);

    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addWidget(cleanupButton);
    buttonLayout->addWidget(vacuumButton);
    buttonLayout->addStretch();

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch();

    connect(refreshButton, &QPushButton::clicked,
            this, &DatabaseMaintenancePanel::refreshInfo);

    connect(cleanupButton, &QPushButton::clicked,
            this, &DatabaseMaintenancePanel::cleanupOldData);

    connect(vacuumButton, &QPushButton::clicked,
            this, &DatabaseMaintenancePanel::vacuumDatabase);
}

//刷新数据库维护面板上的实时信息
void DatabaseMaintenancePanel::refreshInfo()
{
    if (!databaseManager) {
        return;
    }

    pathLabel->setText(databaseManager->databaseFilePath());
    collectCountLabel->setText(QString::number(databaseManager->tableRowCount("collect_data")));
    alarmCountLabel->setText(QString::number(databaseManager->tableRowCount("alarm_log")));
    packetCountLabel->setText(QString::number(databaseManager->tableRowCount("packet_log")));
}

void DatabaseMaintenancePanel::cleanupOldData()
{
    if (!databaseManager) {
        return;
    }

    const auto result = QMessageBox::question(
        this,
        "Delete Old Data",
        "Delete all records before selected time?"
    );

    if (result != QMessageBox::Yes) {
        return;
    }

    if (databaseManager->deleteDataBefore(cutoffTimeEdit->dateTime())) {
        QMessageBox::information(this, "Delete Old Data", "Cleanup completed.");
        refreshInfo();
    }
}

void DatabaseMaintenancePanel::vacuumDatabase()
{
    if (!databaseManager) {
        return;
    }

    if (databaseManager->vacuum()) {
        QMessageBox::information(this, "Vacuum", "Database vacuum completed.");
        refreshInfo();
    }
}
