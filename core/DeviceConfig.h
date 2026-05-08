#ifndef DEVICECONFIG_H
#define DEVICECONFIG_H

#include <QString>
#include <QMetaType> //通过信号槽传递，或存入 QVariant

enum class ModbusMode{
    Rtu,
    Tcp
};

struct SerialConfig{
    QString portName = "COM13";
    int baudRate = 9600;
    int dataBits = 8;
    QString parity = "None";
    int stopBits = 1;
};

struct TcpConfig{
    QString host = "127.0.0.1";
    int port = 5020;
};

struct DeviceConfig
{
    QString deviceName = "模拟设备 1";
    QString deviceId = "device-001";
    int slaveId = 1;
    int timeout = 1000;
    int retryCount = 3;
    ModbusMode mode = ModbusMode::Tcp;
    SerialConfig serial;
    TcpConfig tcp;
};

Q_DECLARE_METATYPE(DeviceConfig); //Qt 元对象类型的声明宏 ，使 DeviceConfig 可以在 Qt 的信号槽机制中使用，或存储在 QVariant 中。

#endif // DEVICECONFIG_H