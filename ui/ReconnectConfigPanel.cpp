#include "ReconnectConfigPanel.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

ReconnectConfigPanel::ReconnectConfigPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void ReconnectConfigPanel::setupUi()
{
    enabledCheckBox = new QCheckBox(tr("启用自动重连"), this);
    enabledCheckBox->setChecked(true);

    intervalSpinBox = new QSpinBox(this);
    intervalSpinBox->setRange(1000, 60000);
    intervalSpinBox->setSingleStep(500);
    intervalSpinBox->setSuffix(tr(" ms"));
    intervalSpinBox->setValue(3000);

    applyButton = new QPushButton(tr("应用配置"), this);
    resetButton = new QPushButton(tr("恢复默认"), this);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("自动重连"), enabledCheckBox);
    formLayout->addRow(tr("重连间隔"), intervalSpinBox);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(resetButton);
    buttonLayout->addWidget(applyButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch();

    connect(applyButton, &QPushButton::clicked,
            this, &ReconnectConfigPanel::applyConfig);

    connect(resetButton, &QPushButton::clicked,
            this, &ReconnectConfigPanel::resetToDefault);
}

void ReconnectConfigPanel::setInitialConfig(bool enabled, int intervalMs)
{
    enabledCheckBox->setChecked(enabled);
    intervalSpinBox->setValue(intervalMs);
}

void ReconnectConfigPanel::applyConfig()
{
    emit reconnectConfigChanged(
        enabledCheckBox->isChecked(),
        intervalSpinBox->value()
    );
}

void ReconnectConfigPanel::resetToDefault()
{
    enabledCheckBox->setChecked(true);
    intervalSpinBox->setValue(3000); // 默认重连间隔为3000ms

    emit reconnectConfigChanged(true, 3000);
}
