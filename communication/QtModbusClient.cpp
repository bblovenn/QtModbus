#include "QtModbusClient.h"

#include <QtSerialBus/QModbusDataUnit>
#include <QtSerialBus/QModbusDevice>
#include <QtSerialBus/QModbusReply>
#include <QtSerialBus/QModbusRtuSerialClient>
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
    // 每次连接前重建客户端，保证 TCP/RTU 模式切换时底层参数不会残留。
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
        client_ = new QModbusRtuSerialClient(this);
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
            wasConnected_ = true;
            manualDisconnect_ = false;
            emit connected();
        } else if (state == QModbusDevice::UnconnectedState) {
            // 只有非用户主动断开才视为异常断开，避免手动断开触发自动重连。
            const bool unexpected = wasConnected_ && !manualDisconnect_;
            wasConnected_ = false;
            emit disconnected();
            if (unexpected) {
                emit unexpectedDisconnected(config_.deviceId);
            }
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
    // 保存配置，后续读写请求需要从站 ID，异常断开信号也需要设备 ID。
    config_ = config;
    manualDisconnect_ = false;
    wasConnected_ = false;
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

    manualDisconnect_ = true; // 标记为用户主动断开，供 stateChanged 判断是否需要重连。
    client_->disconnectDevice();
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
    connect(reply, &QModbusReply::finished, this, [this, reply, address, value]() {
    if (reply->error() != QModbusDevice::NoError) {
        emit errorOccurred(reply->errorString());
        reply->deleteLater();
        return;
    }

    RegisterWriteResult result;
    result.deviceId = config_.deviceId;
    result.type = RegisterType::HoldingRegister;
    result.address = address;
    result.count = 1;
    result.value = value;
    result.timestamp = QDateTime::currentDateTime();

    emit registerWritten(result);

    reply->deleteLater();
});
}

void QtModbusClient::readHoldingRegisters(int startAddress, int count)
{
    readDataUnit(RegisterType::HoldingRegister,
                 QModbusDataUnit::HoldingRegisters,
                 startAddress,
                 count);
}

void QtModbusClient::readInputRegisters(int startAddress, int count)
{
    readDataUnit(RegisterType::InputRegister,
                 QModbusDataUnit::InputRegisters,
                 startAddress,
                 count);
}

void QtModbusClient::readCoils(int startAddress, int count)
{
    readDataUnit(RegisterType::Coil,
                 QModbusDataUnit::Coils,
                 startAddress,
                 count);
}

void QtModbusClient::readDiscreteInputs(int startAddress, int count)
{
    readDataUnit(RegisterType::DiscreteInput,
                 QModbusDataUnit::DiscreteInputs,
                 startAddress,
                 count);
}

void QtModbusClient::readDataUnit(RegisterType type,
                                  QModbusDataUnit::RegisterType qtType,
                                  int startAddress,
                                  int count)
{
    // 所有读请求走同一个函数，避免 01/02/03/04 功能码结果解析逻辑重复。
    if (!client_ || client_->state() != QModbusDevice::ConnectedState) {
        emit errorOccurred("设备未连接");
        return;
    }

    //构造读取请求，根据寄存器类型和 Qt Modbus 的寄存器类型发起读取请求，减少代码重复。
    QModbusDataUnit unit(qtType, startAddress, count);
    //发送读取请求，指定从站ID，返回一个 QModbusReply 对象用于异步处理结果。
    QModbusReply *reply = client_->sendReadRequest(unit, config_.slaveId);

    if (!reply) {
        emit errorOccurred(client_->errorString());
        return;
    }

    //捕捉reply,type,startAddress等局部变量，等待读取完成后解析结果并发出相应信号。
    connect(reply, &QModbusReply::finished, this, [this, reply, type, startAddress]() {
        if (reply->error() != QModbusDevice::NoError) {
            emit errorOccurred(reply->errorString());
            reply->deleteLater();//延时删除,避免在槽函数内直接删除导致访问冲突。
            return;
        }

        //取出返回的数据单元，构造 RegisterReadResult 结果对象，包含设备ID、寄存器类型、起始地址、寄存器值列表和时间戳。
        const QModbusDataUnit unit = reply->result();

        //自定义result结构体
        RegisterReadResult result;
        result.deviceId = config_.deviceId;
        result.type = type;
        result.startAddress = startAddress;
        result.timestamp = QDateTime::currentDateTime();

        //将寄存器值逐个提取出来，封装成 RegisterValue 结构体，并添加到结果的 values 列表中。
        for (uint i = 0; i < unit.valueCount(); ++i) {
            RegisterValue value;
            value.address = startAddress + static_cast<int>(i);
            value.rawValue = unit.value(i);
            result.values.append(value);
        }

        emit registersRead(result);

        //根据寄存器类型发出不同的信号，保持寄存器读取结果发出 holdingRegistersRead 信号，其他类型发出通用的 registersRead 信号。
        if (type == RegisterType::HoldingRegister) {
            emit holdingRegistersRead(result);
        }

        reply->deleteLater();
    });
}

void QtModbusClient::writeSingleCoil(int address, bool value)
{
    if (!client_ || client_->state() != QModbusDevice::ConnectedState) {
        emit errorOccurred("设备未连接");
        return;
    }

    QModbusDataUnit unit(QModbusDataUnit::Coils, address, 1);
    unit.setValue(0, value ? 1 : 0);

    QModbusReply *reply = client_->sendWriteRequest(unit, config_.slaveId);

    if (!reply) {
        emit errorOccurred(client_->errorString());
        return;
    }

    connect(reply, &QModbusReply::finished, this, [this, reply, address, value]() {
    if (reply->error() != QModbusDevice::NoError) {
        emit errorOccurred(reply->errorString());
        reply->deleteLater();
        return;
    }

    RegisterWriteResult result;
    result.deviceId = config_.deviceId;
    result.type = RegisterType::Coil;
    result.address = address;
    result.value = value ? 1 : 0;
    result.count = 1;
    result.timestamp = QDateTime::currentDateTime();

    emit registerWritten(result);

    reply->deleteLater();
});
}

void QtModbusClient::writeMultipleHoldingRegisters(int startAddress, const QVector<quint16> &values)
{
    // 批量写入必须至少有一个值，否则 Qt 会构造出无效请求。
    if (!client_ || client_->state() != QModbusDevice::ConnectedState) {
        emit errorOccurred("设备未连接");
        return;
    }

    if (values.isEmpty()) {
        emit errorOccurred("写入值不能为空");
        return;
    }

    //构造写入请求：保持寄存器类型，多个寄存器
    QModbusDataUnit unit(QModbusDataUnit::HoldingRegisters, startAddress, values.size());

    //遍历值列表
    for (int i = 0; i < values.size(); ++i) {
        unit.setValue(i, values.at(i));
    }

    //发送写入请求，指定从站ID，返回一个 QModbusReply 对象用于异步处理结果。
    QModbusReply *reply = client_->sendWriteRequest(unit, config_.slaveId);

    if (!reply) {
        emit errorOccurred(client_->errorString());
        return;
    }

    connect(reply, &QModbusReply::finished, this, [this, reply, startAddress, values]() {
        if (reply->error() != QModbusDevice::NoError) {
            emit errorOccurred(reply->errorString());
            reply->deleteLater();
            return;
        }

        RegisterWriteResult result;
        result.deviceId = config_.deviceId;
        result.type = RegisterType::HoldingRegister;
        result.address = startAddress;
        result.value = values.first();
        result.count = values.size();
        result.timestamp = QDateTime::currentDateTime();

        emit registerWritten(result);

        reply->deleteLater();
    });
}

void QtModbusClient::writeMultipleCoils(int startAddress, const QVector<bool> &values)
{
    // 线圈批量写入和保持寄存器类似，只是值域从 quint16 变为 bool。
    if (!client_ || client_->state() != QModbusDevice::ConnectedState) {
        emit errorOccurred("设备未连接");
        return;
    }

    if (values.isEmpty()) {
        emit errorOccurred("写入值不能为空");
        return;
    }

    //构造写入请求：线圈类型，多个线圈
    QModbusDataUnit unit(QModbusDataUnit::Coils, startAddress, values.size());

    for (int i = 0; i < values.size(); ++i) {
        //线圈值在 Modbus 中通常用 1 表示 ON，0 表示 OFF，因此根据布尔值设置对应的寄存器值。
        unit.setValue(i, values.at(i) ? 1 : 0); 
    }

    //发送写入请求，指定从站ID，返回一个 QModbusReply 对象用于异步处理结果。
    QModbusReply *reply = client_->sendWriteRequest(unit, config_.slaveId);

    if (!reply) {
        emit errorOccurred(client_->errorString());
        return;
    }

    connect(reply, &QModbusReply::finished, this, [this, reply, startAddress, values]() {
        if (reply->error() != QModbusDevice::NoError) {//如果写入失败，发出错误信号并删除 reply 对象。
            emit errorOccurred(reply->errorString());
            reply->deleteLater();
            return;
        }

        RegisterWriteResult result;
        result.deviceId = config_.deviceId;
        result.type = RegisterType::Coil;
        result.address = startAddress;
        result.value = values.first() ? 1 : 0;
        result.count = values.size();
        result.timestamp = QDateTime::currentDateTime();

        emit registerWritten(result);

        reply->deleteLater();
    });
}
