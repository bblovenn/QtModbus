#ifndef IMODBUSCLIENT_H
#define IMODBUSCLIENT_H

#include "../core/DeviceConfig.h"
#include "../core/RegisterValue.h"

#include <QObject>
#include <QString>
#include <QVector>

// Modbus 通信抽象接口。UI 和业务层只依赖这个信号槽契约，不直接关心 Qt 的客户端实现。
class IModbusClient : public QObject
{
    Q_OBJECT

public:
    explicit IModbusClient(QObject *parent = nullptr)
        : QObject(parent)
    {

    }

    virtual ~IModbusClient() = default;

public slots:
    // 连接、读写、断开都以槽函数暴露，方便 UI 面板直接通过信号驱动通信层。
    virtual void connectDevice(const DeviceConfig &config) = 0;
    virtual void disconnectDevice() = 0;
    virtual void readHoldingRegisters(int startAddress, int count) = 0;
    virtual void readInputRegisters(int startAddress, int count) = 0;
    //读取线圈(可读写位区域),从startaddress开始读取count个线圈状态
    virtual void readCoils(int startAddress, int count) = 0;
    //读取离散输入(只读位区域),从startaddress开始读取count个离散输入状态
    virtual void readDiscreteInputs(int startAddress, int count) = 0;
    //写单个保持寄存器,将value写入address地址的保持寄存器
    virtual void writeSingleHoldingRegister(int address, quint16 value) = 0;
    //写单个线圈,将value写入address地址的线圈
    virtual void writeSingleCoil(int address, bool value) = 0;
    virtual void writeMultipleHoldingRegisters(int startAddress, const QVector<quint16> &values) = 0;
    virtual void writeMultipleCoils(int startAddress, const QVector<bool> &values) = 0;

signals:
    // 通信层统一向外广播结果，调用方无需区分 TCP/RTU 或具体功能码实现。
    void connected();
    void disconnected();
    void errorOccurred(const QString &message);
    void registersRead(const RegisterReadResult &result);
    //保持寄存器读取结果信号，包含设备ID、寄存器类型、起始地址、寄存器值列表和时间戳
    void holdingRegistersRead(const RegisterReadResult &result);
    void registerWritten(const RegisterWriteResult &result);

};

#endif // IMODBUSCLIENT_H
