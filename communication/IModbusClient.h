#ifndef IMODBUSCLIENT_H
#define IMODBUSCLIENT_H

#include "../core/DeviceConfig.h"
#include "../core/RegisterValue.h"

#include <QObject>
#include <QString>

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
    virtual void connectDevice(const DeviceConfig &config) = 0;
    virtual void disconnectDevice() = 0;
    //从 startAddress 开始，连续读取 count 个保持寄存器
    virtual void readHoldingRegisters(int startAddress, int count) = 0;
    //向指定地址写入单个寄存器的值
    virtual void writeSingleHoldingRegister(int address, quint16 value) = 0;

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString &message); // 错误信号
    void holdingRegistersRead(const RegisterReadResult &result); // 读寄存器结果信号
};

#endif // IMODBUSCLIENT_H