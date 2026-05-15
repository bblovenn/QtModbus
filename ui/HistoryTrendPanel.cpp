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
    // —— 查询条件 ——
    deviceIdEdit = new QLineEdit("device-001", this);

    // 默认查询最近 1 小时
    beginTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(-3600), this);
    beginTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    beginTimeEdit->setCalendarPopup(true);

    endTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime(), this);
    endTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    endTimeEdit->setCalendarPopup(true);

    // —— 操作按钮 ——
    queryButton = new QPushButton("Query", this);
    clearButton = new QPushButton("Clear", this);

    // —— 复用实时曲线组件，但隐藏暂停/清空按钮，由本面板自行管理 ——
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

