QT += widgets serialbus serialport sql

CONFIG += c++17
CONFIG -= app_bundle

TEMPLATE = app
TARGET = Modbus

SOURCES += \
    app/AppSettings.cpp \
    app/DeviceCommandSignalBinder.cpp \
    app/MainWindowSignalBinder.cpp \
    app/PacketLogSignalBinder.cpp \
    app/PacketLogService.cpp \
    app/PollingAlarmSignalBinder.cpp \
    app/RegisterTypeText.cpp \
    app/ReconnectSignalBinder.cpp \
    app/SystemStatusSignalBinder.cpp \
    main.cpp \
    mainwindow.cpp \
    alarm/AlarmManager.cpp \
    communication/QtModbusClient.cpp \
    communication/ReconnectController.cpp \
    core/DeviceConfig.cpp \
    core/RegisterValue.cpp \
    database/DatabaseManager.cpp \
    polling/PollingWorker.cpp \
    ui/AlarmConfigPanel.cpp \
    ui/AlarmHistoryPanel.cpp \
    ui/AlarmPanel.cpp \
    ui/ConnectionPanel.cpp \
    ui/DatabaseMaintenancePanel.cpp \
    ui/HistoryPanel.cpp \
    ui/HistoryTrendPanel.cpp \
    ui/MonitorPanel.cpp \
    ui/PacketHistoryPanel.cpp \
    ui/PacketMonitorPanel.cpp \
    ui/PollingConfigPanel.cpp \
    ui/ReconnectConfigPanel.cpp \
    ui/RegisterPanel.cpp \
    ui/SystemStatusPanel.cpp \
    ui/TrendPanel.cpp

HEADERS += \
    app/AppSettings.h \
    app/DeviceCommandSignalBinder.h \
    app/MainWindowPanels.h \
    app/MainWindowRuntimeState.h \
    app/MainWindowSignalBinder.h \
    app/PacketLogSignalBinder.h \
    app/PacketLogService.h \
    app/PollingAlarmSignalBinder.h \
    app/RegisterTypeText.h \
    app/ReconnectSignalBinder.h \
    app/SystemStatusSignalBinder.h \
    mainwindow.h \
    alarm/AlarmManager.h \
    communication/IModbusClient.h \
    communication/QtModbusClient.h \
    communication/ReconnectController.h \
    core/AlarmRecord.h \
    core/DeviceConfig.h \
    core/EngineeringValue.h \
    core/PacketRecord.h \
    core/RegisterValue.h \
    database/DatabaseManager.h \
    polling/PollingWorker.h \
    ui/AlarmConfigPanel.h \
    ui/AlarmHistoryPanel.h \
    ui/AlarmPanel.h \
    ui/ConnectionPanel.h \
    ui/DatabaseMaintenancePanel.h \
    ui/HistoryPanel.h \
    ui/HistoryTrendPanel.h \
    ui/MonitorPanel.h \
    ui/PacketHistoryPanel.h \
    ui/PacketMonitorPanel.h \
    ui/PollingConfigPanel.h \
    ui/ReconnectConfigPanel.h \
    ui/RegisterPanel.h \
    ui/SystemStatusPanel.h \
    ui/TrendPanel.h

FORMS += \
    mainwindow.ui

macx {
    QMAKE_APPLE_DEVICE_ARCHS = arm64

    # Qt 6.10.3 qyieldcpu.h may call __yield() on Apple Silicon before the
    # ARM ACLE intrinsic declaration is visible when building through qmake.
    # Pre-including arm_acle.h keeps qmake builds compatible with Apple Clang.
    QMAKE_CXXFLAGS += -include arm_acle.h
}
