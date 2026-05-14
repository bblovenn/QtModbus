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

    auto *box = new QGroupBox("System Status", this);
    box->setLayout(grid);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(box);
    mainLayout->addStretch();

    setDisconnected();
    setPollingStopped();
    setLabelText(databaseLabel, "Database", "Not ready");
    setLabelText(lastUpdateLabel, "Last Update", "-");
    setLabelText(temperatureLabel, "Temperature", "-");
    setLabelText(voltageLabel, "Voltage", "-");
    setLabelText(currentLabel, "Current", "-");
    setLabelText(speedLabel, "Speed", "-");
    setLabelText(alarmLabel, "Latest Alarm", "-");
}

void SystemStatusPanel::setLabelText(QLabel *label, const QString &title, const QString &value)
{
    label->setText(QString("%1: %2").arg(title, value));
}

void SystemStatusPanel::setConnecting()
{
    setLabelText(connectionLabel, "Connection", "Connecting");
}

void SystemStatusPanel::setConnected()
{
    setLabelText(connectionLabel, "Connection", "Connected");
}

void SystemStatusPanel::setDisconnected()
{
    setLabelText(connectionLabel, "Connection", "Disconnected");
}

void SystemStatusPanel::setCommunicationError(const QString &message)
{
    setLabelText(connectionLabel, "Connection Error", message);
}

void SystemStatusPanel::setPollingStarted()
{
    setLabelText(pollingLabel, "Polling", "Running");
}

void SystemStatusPanel::setPollingStopped()
{
    setLabelText(pollingLabel, "Polling", "Stopped");
}

void SystemStatusPanel::setDatabaseReady(const QString &path)
{
    setLabelText(databaseLabel, "Database", path);
}

//更新状态面板
void SystemStatusPanel::updateEngineeringValue(const EngineeringValue &value)
{
    setLabelText(lastUpdateLabel, "Last Update",
                 value.timestamp.toString("yyyy-MM-dd HH:mm:ss"));

    setLabelText(temperatureLabel, "Temperature",
                 QString("%1 C").arg(value.temperature, 0, 'f', 1));

    setLabelText(voltageLabel, "Voltage",
                 QString("%1 V").arg(value.voltage, 0, 'f', 2));

    setLabelText(currentLabel, "Current",
                 QString("%1 A").arg(value.current, 0, 'f', 2));

    setLabelText(speedLabel, "Speed",
                 QString("%1 rpm").arg(value.speed, 0, 'f', 0));
}

void SystemStatusPanel::updateAlarm(const AlarmRecord &alarm)
{
    setLabelText(alarmLabel, "Latest Alarm", alarm.message);
}
