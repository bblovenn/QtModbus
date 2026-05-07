#ifndef QTMODBUSCLIENT_H
#define QTMODBUSCLIENT_H

#include "IModbusClient.h"

class QModbusClient;

class QtModbusClient : public IModbusClient
{
    Q_OBJECT

public:
    explicit QtModbusClient(QObject *parent = nullptr);
    ~QtModbusClient() override;

public slots:
    void connectDevice(const DeviceConfig &config) override;
    void disconnectDevice() override;
    void readHoldingRegisters(int startAddress, int count) override; //	读取保持寄存器
    void writeSingleHoldingRegister(int address, quint16 value) override; 	//写单个保持寄存器

private:
    void createClient(const DeviceConfig &config); //根据 config.mode 选择创建 RTU 或 TCP 客户端

private:
    DeviceConfig config_;//保存当前连接配置（用于重连等）
    QModbusClient *client_ = nullptr;//指向 Qt Modbus 客户端实例（RAII 风格初始化为 nullptr）
};

#endif // QTMODBUSCLIENT_H