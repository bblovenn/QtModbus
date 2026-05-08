#include "ConnectionPanel.h"

#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

ConnectionPanel::ConnectionPanel(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void ConnectionPanel::setupUi()
{
    modeCombo = new QComboBox(this);
    modeCombo->addItem("Modbus TCP", static_cast<int>(ModbusMode::Tcp));
    modeCombo->addItem("Modbus RTU", static_cast<int>(ModbusMode::Rtu));

    serialPortCombo = new QComboBox(this);
    serialPortCombo->addItems({"COM1", "COM2", "COM3", "COM13"});

    baudRateCombo = new QComboBox(this);
    baudRateCombo->addItems({"9600", "19200", "38400", "57600", "115200"});

    hostEdit = new QLineEdit("127.0.0.1", this);

    portSpin = new QSpinBox(this);
    portSpin->setRange(1, 65535);
    portSpin->setValue(5020);

    slaveIdSpin = new QSpinBox(this);
    slaveIdSpin->setRange(1, 247);
    slaveIdSpin->setValue(1);

    connectButton = new QPushButton("连接", this);
    disconnectButton = new QPushButton("断开", this);

    auto *form = new QFormLayout;
    form->addRow("通信模式", modeCombo);
    form->addRow("串口号", serialPortCombo);
    form->addRow("波特率", baudRateCombo);
    form->addRow("IP 地址", hostEdit);
    form->addRow("端口", portSpin);
    form->addRow("从站地址", slaveIdSpin);

    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(connectButton);
    buttonLayout->addWidget(disconnectButton);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(form);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch();

    connect(connectButton, &QPushButton::clicked, this, [this]() {
        emit connectRequested(currentConfig());
    });

    connect(disconnectButton, &QPushButton::clicked,
            this, &ConnectionPanel::disconnectRequested);
}

//获取当前界面上配置的连接参数并封装成DeviceConfig结构体返回
DeviceConfig ConnectionPanel::currentConfig() const
{
    DeviceConfig config;
    config.mode = static_cast<ModbusMode>(modeCombo->currentData().toInt()); //获取当前选择的通信模式
    config.serial.portName = serialPortCombo->currentText();
    config.serial.baudRate = baudRateCombo->currentText().toInt();
    config.tcp.host = hostEdit->text();
    config.tcp.port = portSpin->value();
    config.slaveId = slaveIdSpin->value();  //获取从站地址

    return config;
}
