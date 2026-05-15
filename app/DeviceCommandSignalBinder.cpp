#include "DeviceCommandSignalBinder.h"

#include "AppSettings.h"
#include "RegisterTypeText.h"
#include "../communication/QtModbusClient.h"
#include "../core/DeviceConfig.h"
#include "../core/RegisterValue.h"
#include "../ui/ConnectionPanel.h"
#include "../ui/RegisterPanel.h"

#include <QMainWindow>
#include <QStatusBar>

void DeviceCommandSignalBinder::bind(const MainWindowSignalBinderContext &context)
{
    // 设备命令域：把用户操作转发到通信层，并把通信结果反馈到状态栏/调试面板。
    QMainWindow *window = context.window;
    AppSettings *settings = context.settings;
    MainWindowPanels *panels = context.panels;
    QtModbusClient *modbusClient = context.modbusClient;

    // —— 连接/断开命令：面板 → 通信层 ——
    QObject::connect(panels->connection, &ConnectionPanel::connectRequested,
        modbusClient, &QtModbusClient::connectDevice
    );

    QObject::connect(panels->connection, &ConnectionPanel::disconnectRequested,
        modbusClient, &QtModbusClient::disconnectDevice
    );

    // —— 状态栏反馈：连接请求 ——
    QObject::connect(panels->connection, &ConnectionPanel::connectRequested,
        window, [window](const DeviceConfig &config) {
            window->statusBar()->showMessage(
                "请求连接：" + config.tcp.host + ":" + QString::number(config.tcp.port)
            );
    });

    QObject::connect(panels->connection, &ConnectionPanel::disconnectRequested,
        window, [window]() {
            window->statusBar()->showMessage("已断开");
    });

    // —— 连接时自动保存配置 ——
    QObject::connect(panels->connection, &ConnectionPanel::connectRequested,
        window, [settings](const DeviceConfig &config) {
            settings->saveDeviceConfig(config);
    });

    // —— 连接/断开/错误状态 → 状态栏 ——
    QObject::connect(modbusClient, &QtModbusClient::connected,
        window, [window]() {
            window->statusBar()->showMessage("已连接");
    });

    QObject::connect(modbusClient, &QtModbusClient::disconnected,
        window, [window]() {
            window->statusBar()->showMessage("已断开");
    });

    QObject::connect(modbusClient, &QtModbusClient::errorOccurred,
        window, [window](const QString &message) {
            window->statusBar()->showMessage("通信错误：" + message);
    });

    // —— 寄存器读写命令：调试面板 → 通信层 ——
    QObject::connect(panels->registers, &RegisterPanel::readHoldingRegistersRequested,
        modbusClient, &QtModbusClient::readHoldingRegisters
    );

    QObject::connect(panels->registers, &RegisterPanel::readInputRegistersRequested,
        modbusClient, &QtModbusClient::readInputRegisters
    );

    QObject::connect(panels->registers, &RegisterPanel::readCoilsRequested,
        modbusClient, &QtModbusClient::readCoils
    );

    QObject::connect(panels->registers, &RegisterPanel::readDiscreteInputsRequested,
        modbusClient, &QtModbusClient::readDiscreteInputs
    );

    QObject::connect(panels->registers, &RegisterPanel::writeSingleHoldingRegisterRequested,
        modbusClient, &QtModbusClient::writeSingleHoldingRegister
    );

    QObject::connect(panels->registers, &RegisterPanel::writeSingleCoilRequested,
        modbusClient, &QtModbusClient::writeSingleCoil
    );

    QObject::connect(panels->registers, &RegisterPanel::writeMultipleHoldingRegistersRequested,
        modbusClient, &QtModbusClient::writeMultipleHoldingRegisters
    );

    QObject::connect(panels->registers, &RegisterPanel::writeMultipleCoilsRequested,
        modbusClient, &QtModbusClient::writeMultipleCoils
    );

    // —— 读结果 → 寄存器调试面板显示 ——
    QObject::connect(modbusClient, &QtModbusClient::registersRead,
        panels->registers, &RegisterPanel::displayRegisters
    );

    // —— 写结果 → 状态栏 ——
    QObject::connect(modbusClient, &QtModbusClient::registerWritten,
        window, [window](const RegisterWriteResult &result) {
            window->statusBar()->showMessage(
                QString("%1 写入成功：地址=%2,值=%3")
                    .arg(registerTypeText(result.type))
                    .arg(result.address)
                    .arg(result.value)
            );
    });
}
