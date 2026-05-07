#include "MonitorPanel.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

MonitorPanel::MonitorPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    resetStats();
}

void MonitorPanel::setupUi()
{
    timestampLabel = new QLabel("暂无数据", this);

    pauseButton = new QPushButton("暂停", this);
    clearButton = new QPushButton("清空", this);

    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(timestampLabel);
    buttonLayout->addStretch();
    buttonLayout->addWidget(pauseButton);
    buttonLayout->addWidget(clearButton);

    table = new QTableWidget(this);
    table->setColumnCount(5);
    table->setRowCount(4);
    table->setHorizontalHeaderLabels({"名称", "当前值", "最小值", "最大值", "平均值"});
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setStretchLastSection(true);

    table->setItem(0, 0, new QTableWidgetItem("温度 ℃"));
    table->setItem(1, 0, new QTableWidgetItem("电压 V"));
    table->setItem(2, 0, new QTableWidgetItem("电流 A"));
    table->setItem(3, 0, new QTableWidgetItem("转速 rpm"));

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(table);

    connect(pauseButton, &QPushButton::clicked,
            this, &MonitorPanel::togglePaused);

    connect(clearButton, &QPushButton::clicked,
            this, &MonitorPanel::clear);
}

void MonitorPanel::updateValue(const EngineeringValue &value)
{
    if (paused) {
        return;
    }

    timestampLabel->setText("更新时间：" + value.timestamp.toString("yyyy-MM-dd HH:mm:ss"));

    updateStats(
        value.temperature,
        value.voltage,
        value.current,
        value.speed
    );

    updateTable();
}

void MonitorPanel::clear()
{
    resetStats();
    timestampLabel->setText("暂无数据");
    updateTable();
}

void MonitorPanel::togglePaused()
{
    paused = !paused;
    pauseButton->setText(paused ? "继续" : "暂停");
}

void MonitorPanel::resetStats()
{
    sampleCount = 0;

    currentTemperature = 0.0;
    currentVoltage = 0.0;
    currentCurrent = 0.0;
    currentSpeed = 0.0;

    minTemperature = 0.0;
    minVoltage = 0.0;
    minCurrent = 0.0;
    minSpeed = 0.0;

    maxTemperature = 0.0;
    maxVoltage = 0.0;
    maxCurrent = 0.0;
    maxSpeed = 0.0;

    sumTemperature = 0.0;
    sumVoltage = 0.0;
    sumCurrent = 0.0;
    sumSpeed = 0.0;
}

void MonitorPanel::updateStats(double temperature, double voltage, double current, double speed)
{
    ++sampleCount;

    currentTemperature = temperature;
    currentVoltage = voltage;
    currentCurrent = current;
    currentSpeed = speed;

    if (sampleCount == 1) {
        minTemperature = maxTemperature = temperature;
        minVoltage = maxVoltage = voltage;
        minCurrent = maxCurrent = current;
        minSpeed = maxSpeed = speed;
    } else {
        minTemperature = qMin(minTemperature, temperature);
        maxTemperature = qMax(maxTemperature, temperature);

        minVoltage = qMin(minVoltage, voltage);
        maxVoltage = qMax(maxVoltage, voltage);

        minCurrent = qMin(minCurrent, current);
        maxCurrent = qMax(maxCurrent, current);

        minSpeed = qMin(minSpeed, speed);
        maxSpeed = qMax(maxSpeed, speed);
    }

    sumTemperature += temperature;
    sumVoltage += voltage;
    sumCurrent += current;
    sumSpeed += speed;
}

void MonitorPanel::updateTable()
{
    const double divisor = sampleCount > 0 ? sampleCount : 1;

    table->setItem(0, 1, new QTableWidgetItem(QString::number(currentTemperature, 'f', 1)));
    table->setItem(0, 2, new QTableWidgetItem(QString::number(minTemperature, 'f', 1)));
    table->setItem(0, 3, new QTableWidgetItem(QString::number(maxTemperature, 'f', 1)));
    table->setItem(0, 4, new QTableWidgetItem(QString::number(sumTemperature / divisor, 'f', 1)));

    table->setItem(1, 1, new QTableWidgetItem(QString::number(currentVoltage, 'f', 2)));
    table->setItem(1, 2, new QTableWidgetItem(QString::number(minVoltage, 'f', 2)));
    table->setItem(1, 3, new QTableWidgetItem(QString::number(maxVoltage, 'f', 2)));
    table->setItem(1, 4, new QTableWidgetItem(QString::number(sumVoltage / divisor, 'f', 2)));

    table->setItem(2, 1, new QTableWidgetItem(QString::number(currentCurrent, 'f', 2)));
    table->setItem(2, 2, new QTableWidgetItem(QString::number(minCurrent, 'f', 2)));
    table->setItem(2, 3, new QTableWidgetItem(QString::number(maxCurrent, 'f', 2)));
    table->setItem(2, 4, new QTableWidgetItem(QString::number(sumCurrent / divisor, 'f', 2)));

    table->setItem(3, 1, new QTableWidgetItem(QString::number(currentSpeed, 'f', 0)));
    table->setItem(3, 2, new QTableWidgetItem(QString::number(minSpeed, 'f', 0)));
    table->setItem(3, 3, new QTableWidgetItem(QString::number(maxSpeed, 'f', 0)));
    table->setItem(3, 4, new QTableWidgetItem(QString::number(sumSpeed / divisor, 'f', 0)));
}
