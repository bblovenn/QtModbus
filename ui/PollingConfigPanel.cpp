#include "PollingConfigPanel.h"

#include <QFormLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

PollingConfigPanel::PollingConfigPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void PollingConfigPanel::setupUi()
{
    // —— 轮询周期：100ms ~ 60s ——
    intervalSpin = new QSpinBox(this);
    intervalSpin->setRange(100, 60000);
    intervalSpin->setSingleStep(100);
    intervalSpin->setSuffix(" ms");

    // —— 起始地址（当前固定为 0，后续可扩展） ——
    startAddressSpin = new QSpinBox(this);
    startAddressSpin->setRange(0, 0);
    startAddressSpin->setValue(0);
    startAddressSpin->setEnabled(false);

    // —— 读取寄存器数量：最少 4 个（温度/电压/电流/转速），最多 125 个 ——
    countSpin = new QSpinBox(this);
    countSpin->setRange(4, 125);
    countSpin->setValue(4);

    applyButton = new QPushButton("Apply", this);
    resetButton = new QPushButton("Reset", this);

    auto *formLayout = new QFormLayout;
    formLayout->addRow("Interval", intervalSpin);
    formLayout->addRow("Start Address", startAddressSpin);
    formLayout->addRow("Register Count", countSpin);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(applyButton);
    mainLayout->addWidget(resetButton);
    mainLayout->addStretch();

    connect(applyButton, &QPushButton::clicked,
            this, &PollingConfigPanel::applyConfig);

    connect(resetButton, &QPushButton::clicked,
            this, &PollingConfigPanel::resetToDefault);
}

void PollingConfigPanel::setInitialConfig(int intervalMs, int startAddress, int count)
{
    intervalSpin->setValue(intervalMs);
    startAddressSpin->setValue(startAddress);
    countSpin->setValue(count);
}

void PollingConfigPanel::applyConfig()
{
    emit pollingConfigChanged(
        intervalSpin->value(),
        startAddressSpin->value(),
        countSpin->value()
    );
}

void PollingConfigPanel::resetToDefault()
{
    intervalSpin->setValue(1000);
    startAddressSpin->setValue(0);
    countSpin->setValue(4);

    applyConfig();
}
