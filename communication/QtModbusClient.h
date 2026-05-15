#ifndef QTMODBUSCLIENT_H
#define QTMODBUSCLIENT_H

#include "IModbusClient.h"
// 引入 Qt Modbus 数据单元头文件：提供 QModbusDataUnit 类，
// 用于封装 Modbus 数据请求的寄存器类型、起始地址和长度
#include <QtSerialBus/QModbusDataUnit>

class QModbusClient;

// 基于 Qt SerialBus 的真实通信实现，负责把项目内部请求映射为 Qt Modbus 请求。
class QtModbusClient : public IModbusClient
{
    Q_OBJECT

public:
    explicit QtModbusClient(QObject *parent = nullptr);
    ~QtModbusClient() override;

public slots:
    void connectDevice(const DeviceConfig &config) override;
    void disconnectDevice() override;
    //读取保持寄存器,从startaddress开始读取count个寄存器值
    void readHoldingRegisters(int startAddress, int count) override;
    void readInputRegisters(int startAddress, int count) override;
    void readCoils(int startAddress, int count) override;
    void readDiscreteInputs(int startAddress, int count) override;
    void writeSingleHoldingRegister(int address, quint16 value) override;
    void writeSingleCoil(int address, bool value) override;
    void writeMultipleHoldingRegisters(int startAddress, const QVector<quint16> &values) override;
    void writeMultipleCoils(int startAddress, const QVector<bool> &values) override;

signals:
    //意外断开连接信号，包含设备ID，供重连控制器使用以区分不同设备的连接状态
    void unexpectedDisconnected(const QString &deviceId);

private:
    // 创建 Modbus 客户端实例，支持 RTU 和 TCP 模式，根据配置参数选择合适的客户端类型。
    void createClient(const DeviceConfig &config);
    // 统一的读取函数，根据寄存器类型和 Qt Modbus 的寄存器类型发起读取请求，减少代码重复。  
    void readDataUnit(RegisterType type, //自定义枚举
                      QModbusDataUnit::RegisterType qtType, //映射到 Qt Modbus 框架 
                      int startAddress,
                      int count
                    );

private:
    // 当前连接配置会保留给读写请求和异常断开通知使用。
    DeviceConfig config_;
    QModbusClient *client_ = nullptr;
    bool manualDisconnect_ = false;
    bool wasConnected_ = false;
};

#endif // QTMODBUSCLIENT_H
