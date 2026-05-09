#include "AlarmHistoryPanel.h"
#include "../database/DatabaseManager.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

AlarmHistoryPanel::AlarmHistoryPanel(DatabaseManager *databaseManagerValue, QWidget *parent)
    : QWidget(parent)
    , databaseManager(databaseManagerValue)
{
    setupUi();
}

void AlarmHistoryPanel::setupUi()
{
    deviceIdEdit = new QLineEdit("device-001", this);

    // 开始时间：当前时间往前1小时，支持日历选择
    beginTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(-3600), this);
    beginTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    beginTimeEdit->setCalendarPopup(true);

    endTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime(), this);
    endTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    endTimeEdit->setCalendarPopup(true);

    // 确认状态下拉框：显示文本 + 实际值（UserRole）
    confirmedCombo = new QComboBox(this);
    confirmedCombo->addItem("All", -1);
    confirmedCombo->addItem("Unconfirmed", 0);
    confirmedCombo->addItem("Confirmed", 1);

    queryButton = new QPushButton("查询", this);

    auto *filterLayout = new QHBoxLayout;
    filterLayout->addWidget(deviceIdEdit);
    filterLayout->addWidget(beginTimeEdit);
    filterLayout->addWidget(endTimeEdit);
    filterLayout->addWidget(confirmedCombo);
    filterLayout->addWidget(queryButton);

    table = new QTableWidget(this);
    table->setColumnCount(10);
    table->setHorizontalHeaderLabels({
        "Time", "Device", "Type", "Level", "Current",
        "Threshold", "Message", "Confirmed", "Confirmed Time", "Alarm ID"
    });
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(filterLayout);
    mainLayout->addWidget(table);

    connect(queryButton, &QPushButton::clicked,
            this, &AlarmHistoryPanel::queryHistory);
}

void AlarmHistoryPanel::queryHistory()
{
    if (!databaseManager) {
        return;
    }

    // 获取当前选择的确认状态过滤值（-1/0/1）
    const int confirmedFilter = confirmedCombo->currentData().toInt();

    const QList<AlarmRecord> records =
        databaseManager->queryAlarmRecords(
            deviceIdEdit->text().trimmed(),
            beginTimeEdit->dateTime(),
            endTimeEdit->dateTime(),
            confirmedFilter
        );

    displayRecords(records);
}

void AlarmHistoryPanel::displayRecords(const QList<AlarmRecord> &records)
{
    table->setRowCount(0);

    for (const AlarmRecord &record : records) {
        const int row = table->rowCount();
        table->insertRow(row);

        table->setItem(row, 0, new QTableWidgetItem(record.alarmTime.toString("yyyy-MM-dd HH:mm:ss")));
        table->setItem(row, 1, new QTableWidgetItem(record.deviceId));
        table->setItem(row, 2, new QTableWidgetItem(alarmTypeText(record.type)));
        table->setItem(row, 3, new QTableWidgetItem(alarmLevelText(record.level)));
        table->setItem(row, 4, new QTableWidgetItem(QString::number(record.currentValue, 'f', 2)));
        table->setItem(row, 5, new QTableWidgetItem(QString::number(record.thresholdValue, 'f', 2)));
        table->setItem(row, 6, new QTableWidgetItem(record.message));
        table->setItem(row, 7, new QTableWidgetItem(record.confirmed ? "Yes" : "No"));
        table->setItem(row, 8, new QTableWidgetItem(
            record.confirmedTime.isValid()
                ? record.confirmedTime.toString("yyyy-MM-dd HH:mm:ss")
                : ""
        ));
        table->setItem(row, 9, new QTableWidgetItem(record.id));
    }
}

// 报警类型枚举转中文字符串
QString AlarmHistoryPanel::alarmTypeText(AlarmType type) const
{
    switch (type) {
    case AlarmType::TemperatureHigh:
        return "温度过高";
    case AlarmType::VoltageLow:
        return "电压过低";
    case AlarmType::CommunicationTimeout:
        return "通信超时";
    case AlarmType::DeviceOffline:
        return "设备离线";
    case AlarmType::ModbusException:
        return "Modbus异常";
    }

    return "未知";
}

// 报警等级枚举转中文字符串
QString AlarmHistoryPanel::alarmLevelText(AlarmLevel level) const
{
    switch (level) {
    case AlarmLevel::Info:
        return "提示";
    case AlarmLevel::Warning:
        return "警告";
    case AlarmLevel::Critical:
        return "严重";
    }

    return "未知";
}
