#include "AlarmConfigPanel.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QPushButton>
#include <QVBoxLayout>

AlarmConfigPanel::AlarmConfigPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void AlarmConfigPanel::setupUi()
{
    // —— 温度上限阈值 ——
    temperatureHighSpin = new QDoubleSpinBox(this);
    temperatureHighSpin->setRange(-1000.0, 1000.0);
    temperatureHighSpin->setDecimals(1);
    temperatureHighSpin->setSingleStep(1.0);
    temperatureHighSpin->setSuffix(" C");

    // —— 电压下限阈值 ——
    voltageLowSpin = new QDoubleSpinBox(this);
    voltageLowSpin->setRange(0.0, 1000.0);
    voltageLowSpin->setDecimals(2);
    voltageLowSpin->setSingleStep(0.5);
    voltageLowSpin->setSuffix(" V");

    applyButton = new QPushButton("应用", this);

    auto *formLayout = new QFormLayout;
    formLayout->addRow("最高温度", temperatureHighSpin);
    formLayout->addRow("最低电压", voltageLowSpin);

    resetButton = new QPushButton("重置", this);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(applyButton);
    mainLayout->addWidget(resetButton);
    mainLayout->addStretch();

    connect(applyButton, &QPushButton::clicked,
            this, &AlarmConfigPanel::applyLimits);

    connect(resetButton, &QPushButton::clicked,
            this, &AlarmConfigPanel::resetToDefault);
}

void AlarmConfigPanel::setInitialLimits(double temperatureHighLimit, double voltageLowLimit)
{
    temperatureHighSpin->setValue(temperatureHighLimit);
    voltageLowSpin->setValue(voltageLowLimit);
}

void AlarmConfigPanel::applyLimits()
{
    // 将当前界面阈值通过信号发出，由 Binder 层负责同步到 AlarmManager 并持久化
    emit alarmLimitsChanged(
        temperatureHighSpin->value(),
        voltageLowSpin->value()
    );
}

void AlarmConfigPanel::resetToDefault()
{
    temperatureHighSpin->setValue(80.0);
    voltageLowSpin->setValue(18.0);

    applyLimits();
}
