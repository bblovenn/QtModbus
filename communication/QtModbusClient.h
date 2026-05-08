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
    void readHoldingRegisters(int startAddress, int count) override;
    void writeSingleHoldingRegister(int address, quint16 value) override;

signals:
    void unexpectedDisconnected(const QString &deviceId);

private:
    void createClient(const DeviceConfig &config);

private:
    DeviceConfig config_;
    QModbusClient *client_ = nullptr;
    bool manualDisconnect_ = false;
    bool wasConnected_ = false;
};

#endif // QTMODBUSCLIENT_H
