#include "SystemStatusPanel.h"

#include <QDateTime>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

SystemStatusPanel::SystemStatusPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void SystemStatusPanel::setupUi()
{
    connectionLabel = new QLabel(this);
    pollingLabel = new QLabel(this);
    databaseLabel = new QLabel(this);
    lastUpdateLabel = new QLabel(this);
    temperatureLabel = new QLabel(this);
    voltageLabel = new QLabel(this);
    currentLabel = new QLabel(this);
    speedLabel = new QLabel(this);
    alarmLabel = new QLabel(this);

    auto *grid = new QGridLayout;
    grid->addWidget(connectionLabel, 0, 0);
    grid->addWidget(pollingLabel, 0, 1);
    grid->addWidget(databaseLabel, 1, 0, 1, 2);
    grid->addWidget(lastUpdateLabel, 2, 0, 1, 2);
    grid->addWidget(temperatureLabel, 3, 0);
    grid->addWidget(voltageLabel, 3, 1);
    grid->addWidget(currentLabel, 4, 0);
    grid->addWidget(speedLabel, 4, 1);
    grid->addWidget(alarmLabel, 5, 0, 1, 2);

    auto *box = new QGroupBox("系统状态", this);
    box->setLayout(grid);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(box);
    mainLayout->addStretch();

    setDisconnected();
    setPollingStopped();
    setLabelText(databaseLabel, "数据库", "未就绪");
    setLabelText(lastUpdateLabel, "最后更新", "-");
    setLabelText(temperatureLabel, "温度", "-");
    setLabelText(voltageLabel, "电压", "-");
    setLabelText(currentLabel, "电流", "-");
    setLabelText(speedLabel, "转速", "-");
    setLabelText(alarmLabel, "最新报警", "-");
}

void SystemStatusPanel::setLabelText(QLabel *label, const QString &title, const QString &value)
{
    label->setText(QString("%1: %2").arg(title, value));
}

void SystemStatusPanel::setConnecting()
{
    setLabelText(connectionLabel, "连接", "连接中");
}

void SystemStatusPanel::setConnected()
{
    setLabelText(connectionLabel, "连接", "已连接");
}

void SystemStatusPanel::setDisconnected()
{
    setLabelText(connectionLabel, "连接", "已断开");
}

void SystemStatusPanel::setCommunicationError(const QString &message)
{
    setLabelText(connectionLabel, "连接错误", message);
}

void SystemStatusPanel::setPollingStarted()
{
    setLabelText(pollingLabel, "轮询", "运行中");
}

void SystemStatusPanel::setPollingStopped()
{
    setLabelText(pollingLabel, "轮询", "已停止");
}

void SystemStatusPanel::setDatabaseReady(const QString &path)
{
    setLabelText(databaseLabel, "数据库", path);
}

// 收到新一轮轮询结果时，刷新状态总览中的实时数据和最后更新时间
void SystemStatusPanel::updateEngineeringValue(const EngineeringValue &value)
{
    // 状态总览只展示最新一帧工程量，完整历史由历史查询页负责。
    setLabelText(lastUpdateLabel, "最后更新",
                 value.timestamp.toString("yyyy-MM-dd HH:mm:ss"));

    setLabelText(temperatureLabel, "温度",
                 QString("%1 ℃").arg(value.temperature, 0, 'f', 1));

    setLabelText(voltageLabel, "电压",
                 QString("%1 V").arg(value.voltage, 0, 'f', 2));

    setLabelText(currentLabel, "电流",
                 QString("%1 A").arg(value.current, 0, 'f', 2));

    setLabelText(speedLabel, "转速",
                 QString("%1 rpm").arg(value.speed, 0, 'f', 0));
}

void SystemStatusPanel::updateAlarm(const AlarmRecord &alarm)
{
    // 这里只保留最新报警摘要，报警列表和确认操作在报警面板中完成。
    setLabelText(alarmLabel, "最新报警", alarm.message);
}
