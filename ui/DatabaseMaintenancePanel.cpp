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

    refreshButton = new QPushButton("刷新", this);
    cleanupButton = new QPushButton("删除此时间之前的数据", this);
    vacuumButton = new QPushButton("压缩数据库", this);

    auto *formLayout = new QFormLayout;
    formLayout->addRow("数据库路径", pathLabel);
    formLayout->addRow("采集数据行数", collectCountLabel);
    formLayout->addRow("报警数据行数", alarmCountLabel);
    formLayout->addRow("报文数据行数", packetCountLabel);
    formLayout->addRow("截止时间", cutoffTimeEdit);

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
    // 不缓存统计信息，每次刷新都读取数据库当前状态。
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
    // 删除 cutoffTime 之前的所有记录（collect_data / alarm_log / packet_log），需用户确认
    if (!databaseManager) {
        return;
    }

    const auto result = QMessageBox::question(
        this,
        "删除旧数据",
        "删除选中时间之前的所有记录？"
    );

    if (result != QMessageBox::Yes) {
        return;
    }

    if (databaseManager->deleteDataBefore(cutoffTimeEdit->dateTime())) {
        QMessageBox::information(this, "删除旧数据", "清理完成。");
        refreshInfo();
    }
}

void DatabaseMaintenancePanel::vacuumDatabase()
{
    if (!databaseManager) {
        return;
    }

    if (databaseManager->vacuum()) {
        QMessageBox::information(this, "压缩数据库", "数据库压缩完成。");
        refreshInfo();
    }
}
