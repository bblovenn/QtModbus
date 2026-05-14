#include "HistoryTrendPanel.h"
#include "TrendPanel.h"
#include "../database/DatabaseManager.h"

#include <QDateTime>
#include <QDateTimeEdit>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QVector>

HistoryTrendPanel::HistoryTrendPanel(DatabaseManager *databaseManagerValue, QWidget *parent)
    : QWidget(parent)
    , databaseManager(databaseManagerValue)
{
    setupUi();
}

void HistoryTrendPanel::setupUi()
{
    deviceIdEdit = new QLineEdit("device-001", this);

    beginTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(-3600), this);
    beginTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    beginTimeEdit->setCalendarPopup(true);

    endTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime(), this);
    endTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    endTimeEdit->setCalendarPopup(true);

    queryButton = new QPushButton("Query", this);
    clearButton = new QPushButton("Clear", this);
    trendPanel = new TrendPanel(this);
    trendPanel->setControlPanelVisible(false);

    auto *formLayout = new QFormLayout;
    formLayout->addRow("Device ID", deviceIdEdit);
    formLayout->addRow("Begin Time", beginTimeEdit);
    formLayout->addRow("End Time", endTimeEdit);

    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(queryButton);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addStretch();

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(trendPanel, 1);

    connect(queryButton, &QPushButton::clicked,
            this, &HistoryTrendPanel::queryTrend);

    connect(clearButton, &QPushButton::clicked,
            this, &HistoryTrendPanel::clearTrend);
}

void HistoryTrendPanel::queryTrend()
{
    if (!databaseManager) {
        return;
    }

    const auto values = databaseManager->queryEngineeringValues(
        deviceIdEdit->text().trimmed(),
        beginTimeEdit->dateTime(),
        endTimeEdit->dateTime()
    );

    trendPanel->setValues(QVector<EngineeringValue>::fromList(values));
}

void HistoryTrendPanel::clearTrend()
{
    trendPanel->setValues(QVector<EngineeringValue>());
}

