#include "PacketLogSignalBinder.h"

#include "PacketLogService.h"
#include "RegisterTypeText.h"
#include "../alarm/AlarmManager.h"
#include "../communication/QtModbusClient.h"
#include "../core/AlarmRecord.h"
#include "../core/DeviceConfig.h"
#include "../core/RegisterValue.h"
#include "../ui/ConnectionPanel.h"
#include "../ui/PacketMonitorPanel.h"
#include "../ui/RegisterPanel.h"

void PacketLogSignalBinder::bind(const MainWindowSignalBinderContext &context)
{
    // 所有关键通信动作都写入同一条日志通道，便于联调时按时间线排查问题。
    MainWindowPanels *panels = context.panels;
    PacketLogService *packetLog = context.packetLog;
    QtModbusClient *modbusClient = context.modbusClient;
    AlarmManager *alarmManager = context.alarmManager;

    // —— 连接事件日志 ——
    QObject::connect(panels->connection, &ConnectionPanel::connectRequested,
        panels->packets, [packetLog](const DeviceConfig &config) {
            packetLog->append(
                "Connection",
                "TX",
                QString("连接请求 %1:%2 从站=%3")
                    .arg(config.tcp.host)
                    .arg(config.tcp.port)
                    .arg(config.slaveId)
            );
    });

    QObject::connect(panels->connection, &ConnectionPanel::disconnectRequested,
        panels->packets, [packetLog]() {
            packetLog->append(
                "Connection",
                "TX",
                "断开连接请求"
            );
    });

    // —— 读请求日志 ——
    QObject::connect(panels->registers, &RegisterPanel::readHoldingRegistersRequested,
        panels->packets, [packetLog](int startAddress, int count) {
            packetLog->append(
                "Modbus",
                "TX",
                QString("读取保持寄存器 起始=%1 数量=%2")
                    .arg(startAddress)
                    .arg(count)
            );
    });

    QObject::connect(panels->registers, &RegisterPanel::readInputRegistersRequested,
        panels->packets, [packetLog](int startAddress, int count) {
            packetLog->append(
                "Modbus",
                "TX",
                QString("读取输入寄存器 起始=%1 数量=%2")
                    .arg(startAddress)
                    .arg(count)
            );
    });

    QObject::connect(panels->registers, &RegisterPanel::readCoilsRequested,
        panels->packets, [packetLog](int startAddress, int count) {
            packetLog->append(
                "Modbus",
                "TX",
                QString("读取线圈 起始=%1 数量=%2")
                    .arg(startAddress)
                    .arg(count)
            );
    });

    QObject::connect(panels->registers, &RegisterPanel::readDiscreteInputsRequested,
        panels->packets, [packetLog](int startAddress, int count) {
            packetLog->append(
                "Modbus",
                "TX",
                QString("读取离散输入 起始=%1 数量=%2")
                    .arg(startAddress)
                    .arg(count)
            );
    });

    // —— 写请求日志 ——
    QObject::connect(panels->registers, &RegisterPanel::writeSingleHoldingRegisterRequested,
        panels->packets, [packetLog](int address, quint16 value) {
            packetLog->append(
                "Modbus",
                "TX",
                QString("写单个保持寄存器 地址=%1 值=%2")
                    .arg(address)
                    .arg(value)
            );
    });

    QObject::connect(panels->registers, &RegisterPanel::writeSingleCoilRequested,
        panels->packets, [packetLog](int address, bool value) {
            packetLog->append(
                "Modbus",
                "TX",
                QString("写单个线圈 地址=%1 值=%2")
                    .arg(address)
                    .arg(value ? "开" : "关")
            );
    });

    QObject::connect(panels->registers, &RegisterPanel::writeMultipleHoldingRegistersRequested,
        panels->packets, [packetLog](int startAddress, const QVector<quint16> &values) {
            packetLog->append(
                "Modbus",
                "TX",
                QString("写多个保持寄存器 起始=%1 数量=%2")
                    .arg(startAddress)
                    .arg(values.size())
            );
    });

    QObject::connect(panels->registers, &RegisterPanel::writeMultipleCoilsRequested,
        panels->packets, [packetLog](int startAddress, const QVector<bool> &values) {
            packetLog->append(
                "Modbus",
                "TX",
                QString("写多个线圈 起始=%1 数量=%2")
                    .arg(startAddress)
                    .arg(values.size())
            );
    });

    // —— 通信错误日志 ——
    QObject::connect(modbusClient, &QtModbusClient::errorOccurred,
        panels->packets, [packetLog](const QString &message) {
            packetLog->append(
                "Communication",
                "RX",
                "错误 " + message
            );
    });

    // —— 读/写结果日志 ——
    QObject::connect(modbusClient, &QtModbusClient::registersRead,
        panels->packets, [packetLog](const RegisterReadResult &result) {
            packetLog->append(
                "Modbus",
                "RX",
                QString("%1 读取成功 起始=%2 数量=%3")
                    .arg(registerTypeText(result.type))
                    .arg(result.startAddress)
                    .arg(result.values.size())
            );
    });

    QObject::connect(modbusClient, &QtModbusClient::registerWritten,
        panels->packets, [packetLog](const RegisterWriteResult &result) {
            packetLog->append(
                "Modbus",
                "RX",
                QString("%1 写入成功 地址=%2 数量=%3 值=%4")
                    .arg(registerTypeText(result.type))
                    .arg(result.address)
                    .arg(result.count)
                    .arg(result.value)
            );
    });

    // —— 报警事件日志 ——
    QObject::connect(alarmManager, &AlarmManager::alarmRaised,
        panels->packets, [packetLog](const AlarmRecord &alarm) {
            packetLog->append(
                "Alarm",
                "ALARM",
                alarm.message
            );
    });
}
