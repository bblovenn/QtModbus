cmake_minimum_required(VERSION 3.16)

project(ModbusIndustrialHmi VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt6 ...) call.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets SerialBus SerialPort Sql)

set(MODBUS_SOURCES
  main.cpp
  mainwindow.cpp
  mainwindow.h
  mainwindow.ui
  app/AppSettings.h
  app/AppSettings.cpp
  app/DeviceCommandSignalBinder.h
  app/DeviceCommandSignalBinder.cpp
  app/MainWindowPanels.h
  app/MainWindowRuntimeState.h
  app/MainWindowSignalBinder.h
  app/MainWindowSignalBinder.cpp
  app/PacketLogSignalBinder.h
  app/PacketLogSignalBinder.cpp
  app/PacketLogService.h
  app/PacketLogService.cpp
  app/PollingAlarmSignalBinder.h
  app/PollingAlarmSignalBinder.cpp
  app/RegisterTypeText.h
  app/RegisterTypeText.cpp
  app/ReconnectSignalBinder.h
  app/ReconnectSignalBinder.cpp
  app/SystemStatusSignalBinder.h
  app/SystemStatusSignalBinder.cpp
  core/DeviceConfig.h
  core/RegisterValue.h
  core/EngineeringValue.h
  core/AlarmRecord.h
  core/PacketRecord.h
  ui/ConnectionPanel.h
  ui/ConnectionPanel.cpp
  ui/RegisterPanel.h
  ui/RegisterPanel.cpp
  ui/PacketMonitorPanel.h
  ui/PacketMonitorPanel.cpp
  ui/MonitorPanel.h
  ui/MonitorPanel.cpp
  ui/AlarmPanel.h
  ui/AlarmPanel.cpp
  ui/HistoryPanel.h
  ui/HistoryPanel.cpp
  ui/AlarmHistoryPanel.h
  ui/AlarmHistoryPanel.cpp
  ui/AlarmConfigPanel.h
  ui/AlarmConfigPanel.cpp
  ui/PacketHistoryPanel.h
  ui/PacketHistoryPanel.cpp
  ui/DatabaseMaintenancePanel.h
  ui/DatabaseMaintenancePanel.cpp
  ui/PollingConfigPanel.h
  ui/PollingConfigPanel.cpp
  ui/TrendPanel.h
  ui/TrendPanel.cpp
  ui/HistoryTrendPanel.h
  ui/HistoryTrendPanel.cpp
  ui/SystemStatusPanel.h
  ui/SystemStatusPanel.cpp
  ui/ReconnectConfigPanel.h
  ui/ReconnectConfigPanel.cpp
  communication/IModbusClient.h
  communication/QtModbusClient.h
  communication/QtModbusClient.cpp
  communication/ReconnectController.h
  communication/ReconnectController.cpp
  polling/PollingWorker.h
  polling/PollingWorker.cpp
  alarm/AlarmManager.h
  alarm/AlarmManager.cpp
  database/DatabaseManager.h
  database/DatabaseManager.cpp
)

if(ANDROID)
  add_library(Modbus SHARED ${MODBUS_SOURCES})
else()
  add_executable(Modbus ${MODBUS_SOURCES})
endif()

target_link_libraries(Modbus PRIVATE
    Qt6::Widgets
    Qt6::SerialBus
    Qt6::SerialPort
    Qt6::Sql
)
