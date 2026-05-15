#include "AlarmHistoryPanel.h"
#include "../database/DatabaseManager.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QFile>
#include <QFileDialog>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStringConverter>
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

AlarmHistoryPanel::AlarmHistoryPanel(DatabaseManager *databaseManagerValue, QWidget *parent)
    : QWidget(parent)
    , databaseManager(databaseManagerValue)
{
    setupUi();
}

void AlarmHistoryPanel::setupUi()
{
    deviceIdEdit = new QLineEdit("device-001", this);

    beginTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(-3600), this);
    beginTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    beginTimeEdit->setCalendarPopup(true);

    endTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime(), this);
    endTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    endTimeEdit->setCalendarPopup(true);

    confirmedCombo = new QComboBox(this);
    confirmedCombo->addItem(text(u8"全部"), -1);
    confirmedCombo->addItem(text(u8"未确认"), 0);
    confirmedCombo->addItem(text(u8"已确认"), 1);

    queryButton = new QPushButton(text(u8"查询"), this);
    exportButton = new QPushButton(text(u8"导出 CSV"), this);

    auto *filterLayout = new QHBoxLayout;
    filterLayout->addWidget(deviceIdEdit);
    filterLayout->addWidget(beginTimeEdit);
    filterLayout->addWidget(endTimeEdit);
    filterLayout->addWidget(confirmedCombo);
    filterLayout->addWidget(queryButton);
    filterLayout->addWidget(exportButton);

    table = new QTableWidget(this);
    table->setColumnCount(10);
    table->setHorizontalHeaderLabels({
        text(u8"报警时间"),
        text(u8"设备 ID"),
        text(u8"报警类型"),
        text(u8"报警等级"),
        text(u8"当前值"),
        text(u8"阈值"),
        text(u8"消息"),
        text(u8"确认状态"),
        text(u8"确认时间"),
        text(u8"报警 ID")
    });
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(filterLayout);
    mainLayout->addWidget(table);

    connect(queryButton, &QPushButton::clicked,
            this, &AlarmHistoryPanel::queryHistory);

    connect(exportButton, &QPushButton::clicked,
            this, &AlarmHistoryPanel::exportCsv);
}

void AlarmHistoryPanel::queryHistory()
{
    // confirmedFilter: 全部/未确认/已确认，由下拉框映射为 -1/0/1。
    if (!databaseManager) {
        return;
    }

    endTimeEdit->setDateTime(QDateTime::currentDateTime());

    const int confirmedFilter = confirmedCombo->currentData().toInt();

    currentRecords = databaseManager->queryAlarmRecords(
        deviceIdEdit->text().trimmed(),
        beginTimeEdit->dateTime(),
        endTimeEdit->dateTime(),
        confirmedFilter
    );

    displayRecords(currentRecords);
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
        table->setItem(row, 7, new QTableWidgetItem(record.confirmed ? text(u8"已确认") : text(u8"未确认")));
        table->setItem(row, 8, new QTableWidgetItem(
            record.confirmedTime.isValid()
                ? record.confirmedTime.toString("yyyy-MM-dd HH:mm:ss")
                : ""
        ));
        table->setItem(row, 9, new QTableWidgetItem(record.id));
    }
}

QString AlarmHistoryPanel::alarmTypeText(AlarmType type) const
{
    switch (type) {
    case AlarmType::TemperatureHigh:
        return text(u8"温度过高");
    case AlarmType::VoltageLow:
        return text(u8"电压过低");
    case AlarmType::CommunicationTimeout:
        return text(u8"通信超时");
    case AlarmType::DeviceOffline:
        return text(u8"设备离线");
    case AlarmType::ModbusException:
        return text(u8"Modbus 异常");
    }

    return text(u8"未知");
}

QString AlarmHistoryPanel::alarmLevelText(AlarmLevel level) const
{
    switch (level) {
    case AlarmLevel::Info:
        return text(u8"提示");
    case AlarmLevel::Warning:
        return text(u8"警告");
    case AlarmLevel::Critical:
        return text(u8"严重");
    }

    return text(u8"未知");
}

void AlarmHistoryPanel::exportCsv()
{
    // CSV 导出当前表格对应的数据，便于现场排查后留档。
    if (currentRecords.isEmpty()) {
        QMessageBox::information(this, text(u8"导出 CSV"), text(u8"没有报警记录可以导出。"));
        return;
    }

    const QString fileName = QFileDialog::getSaveFileName(
        this,
        text(u8"导出报警数据"),
        "alarm_log.csv",
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
    out.setEncoding(QStringConverter::Utf8);
    out.setGenerateByteOrderMark(true);

    out << csvEscape(text(u8"报警 ID")) << ","
        << csvEscape(text(u8"报警时间")) << ","
        << csvEscape(text(u8"设备 ID")) << ","
        << csvEscape(text(u8"报警类型")) << ","
        << csvEscape(text(u8"报警等级")) << ","
        << csvEscape(text(u8"当前值")) << ","
        << csvEscape(text(u8"阈值")) << ","
        << csvEscape(text(u8"消息")) << ","
        << csvEscape(text(u8"确认状态")) << ","
        << csvEscape(text(u8"确认时间")) << "\n";

    for (const AlarmRecord &record : currentRecords) {
        out << csvEscape(record.id) << ","
            << csvEscape(record.alarmTime.toString("yyyy-MM-dd HH:mm:ss")) << ","
            << csvEscape(record.deviceId) << ","
            << csvEscape(alarmTypeText(record.type)) << ","
            << csvEscape(alarmLevelText(record.level)) << ","
            << csvEscape(QString::number(record.currentValue, 'f', 2)) << ","
            << csvEscape(QString::number(record.thresholdValue, 'f', 2)) << ","
            << csvEscape(record.message) << ","
            << csvEscape(record.confirmed ? text(u8"已确认") : text(u8"未确认")) << ","
            << csvEscape(record.confirmedTime.isValid()
                    ? record.confirmedTime.toString("yyyy-MM-dd HH:mm:ss")
                    : "")
            << "\n";
    }

    file.close();

    QMessageBox::information(this, text(u8"导出 CSV"), text(u8"导出完成。"));
}
