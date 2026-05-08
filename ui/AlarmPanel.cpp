#include "AlarmPanel.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

AlarmPanel::AlarmPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void AlarmPanel::setupUi()
{
    confirmButton = new QPushButton("确认选中报警", this);
    clearButton = new QPushButton("清空", this);

    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(confirmButton);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addStretch();

    table = new QTableWidget(this);
    table->setColumnCount(9);
    table->setHorizontalHeaderLabels({
        "时间", "设备", "类型", "等级", "当前值",
        "阈值", "消息", "是否确认", "确认时间"
    });
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(table);

    connect(confirmButton, &QPushButton::clicked,
            this, &AlarmPanel::confirmSelectedAlarm);

    connect(clearButton, &QPushButton::clicked,
            this, &AlarmPanel::clear);
}

void AlarmPanel::appendAlarm(const AlarmRecord &alarm)
{
    const int row = table->rowCount();
    table->insertRow(row);

    table->setItem(row, 0, new QTableWidgetItem(alarm.alarmTime.toString("yyyy-MM-dd HH:mm:ss")));
    table->setItem(row, 1, new QTableWidgetItem(alarm.deviceId));
    table->setItem(row, 2, new QTableWidgetItem(alarmTypeText(alarm.type)));
    table->setItem(row, 3, new QTableWidgetItem(alarmLevelText(alarm.level)));
    table->setItem(row, 4, new QTableWidgetItem(QString::number(alarm.currentValue, 'f', 2)));
    table->setItem(row, 5, new QTableWidgetItem(QString::number(alarm.thresholdValue, 'f', 2)));
    table->setItem(row, 6, new QTableWidgetItem(alarm.message));
    table->setItem(row, 7, new QTableWidgetItem("否"));
    table->setItem(row, 8, new QTableWidgetItem(""));
}

void AlarmPanel::confirmSelectedAlarm()
{
    const int row = table->currentRow();

    if (row < 0) {
        return;
    }

    table->setItem(row, 7, new QTableWidgetItem("是"));
    table->setItem(row, 8, new QTableWidgetItem(
        QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
    ));
}

void AlarmPanel::clear()
{
    table->setRowCount(0);
}

QString AlarmPanel::alarmTypeText(AlarmType type) const
{
    switch (type) {
    case AlarmType::TemperatureHigh:
        return "温度过高";
    case AlarmType::VoltageLow:
        return "电压过低";
    case AlarmType::DeviceOffline:
        return "设备离线";
    case AlarmType::CommunicationTimeout:
        return "通信超时";
    case AlarmType::ModbusException:
        return "Modbus异常";
    }

    return "未知";
}

QString AlarmPanel::alarmLevelText(AlarmLevel level) const
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
