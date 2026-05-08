#include "QtModbusClient.h"

#include <QtSerialBus/QModbusDataUnit>
#include <QtSerialBus/QModbusDevice>
#include <QtSerialBus/QModbusReply>
#include <QtSerialBus/QModbusRtuSerialMaster>
#include <QtSerialBus/QModbusTcpClient>
#include <QtSerialPort/QSerialPort>

#include <QVariant>

QtModbusClient::QtModbusClient(QObject *parent)
    : IModbusClient(parent)
{
}

QtModbusClient::~QtModbusClient()
{
    disconnectDevice();
}

void QtModbusClient::createClient(const DeviceConfig &config)
{
    //释放旧的客户端实例
    if (client_) {
        client_->disconnectDevice();
        client_->deleteLater();
        client_ = nullptr;
    }

    if (config.mode == ModbusMode::Tcp) {
        client_ = new QModbusTcpClient(this);
        client_->setConnectionParameter(QModbusDevice::NetworkAddressParameter, config.tcp.host);
        client_->setConnectionParameter(QModbusDevice::NetworkPortParameter, config.tcp.port);
    } else {
        client_ = new QModbusRtuSerialMaster(this);
        client_->setConnectionParameter(QModbusDevice::SerialPortNameParameter, config.serial.portName);
        client_->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, config.serial.baudRate);
        client_->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, QSerialPort::Data8);
        client_->setConnectionParameter(QModbusDevice::SerialParityParameter, QSerialPort::NoParity);
        client_->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, QSerialPort::OneStop);
    }

    // 设置超时和重试次数
    client_->setTimeout(config.timeout);
    client_->setNumberOfRetries(config.retryCount);

    connect(client_, &QModbusClient::stateChanged, this, [this](int state) {
        if (state == QModbusDevice::ConnectedState) {
            emit connected();
        } else if (state == QModbusDevice::UnconnectedState) {
            emit disconnected();
        }
    });

    connect(client_, &QModbusClient::errorOccurred, this, [this]() {
        if (client_) {
            emit errorOccurred(client_->errorString());
        }
    });
}

void QtModbusClient::connectDevice(const DeviceConfig &config)
{
    config_ = config;
    createClient(config_);

    if (!client_->connectDevice()) {
        emit errorOccurred(client_->errorString());
    }
}

void QtModbusClient::disconnectDevice()
{
    if (!client_) {
        return;
    }

    client_->disconnectDevice();
}

// 读取保持寄存器
void QtModbusClient::readHoldingRegisters(int startAddress, int count)
{
    if (!client_ || client_->state() != QModbusDevice::ConnectedState) {
        emit errorOccurred("设备未连接");
        return;
    }

    // 构造读取请求：保持寄存器类型，从 startAddress 开始，读取 count 个
    QModbusDataUnit unit(QModbusDataUnit::HoldingRegisters, startAddress, count);
    QModbusReply *reply = client_->sendReadRequest(unit, config_.slaveId);

    if (!reply) {
        emit errorOccurred(client_->errorString());
        return;
    }

    // 异步处理读取结果
    connect(reply, &QModbusReply::finished, this, [this, reply, startAddress]() {
        if (reply->error() != QModbusDevice::NoError) {
            emit errorOccurred(reply->errorString());
            reply->deleteLater();
            return;
        }

        const QModbusDataUnit unit = reply->result();

        // 解析返回的数据
        RegisterReadResult result;
        result.deviceId = config_.deviceId;
        result.type = RegisterType::HoldingRegister;
        result.startAddress = startAddress;
        result.timestamp = QDateTime::currentDateTime();

        // 将寄存器值逐个提取出来
        for (uint i = 0; i < unit.valueCount(); ++i) {
            RegisterValue value;
            value.address = startAddress + static_cast<int>(i);
            value.rawValue = unit.value(i);
            result.values.append(value);
        }

        emit holdingRegistersRead(result);
        reply->deleteLater();
    });
}

// 写入单个保持寄存器
void QtModbusClient::writeSingleHoldingRegister(int address, quint16 value)
{
    if (!client_ || client_->state() != QModbusDevice::ConnectedState) {
        emit errorOccurred("设备未连接");
        return;
    }

    // 构造写入请求：保持寄存器类型，单个寄存器
    QModbusDataUnit unit(QModbusDataUnit::HoldingRegisters, address, 1);
    unit.setValue(0, value);

    QModbusReply *reply = client_->sendWriteRequest(unit, config_.slaveId);

    if (!reply) {
        emit errorOccurred(client_->errorString());
        return;
    }

    // 异步处理写入结果
    connect(reply, &QModbusReply::finished, this, [this, reply]() {
        if (reply->error() != QModbusDevice::NoError) {
            emit errorOccurred(reply->errorString());
        }

        reply->deleteLater();
    });
}
