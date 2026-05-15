#include "AppSettings.h"

#include <QSettings>

namespace {

// QSettings 的组织名和应用名固定后，配置会在同一个位置持续复用。
constexpr auto kOrganization = "QtModbusHmi";
constexpr auto kApplication = "ModbusIndustrialHmi";

} // namespace

AppSettings::AppSettings() = default;

DeviceConfig AppSettings::loadDeviceConfig() const
{
    // 没有历史配置时使用本地模拟器默认连接参数。
    QSettings settings = createSettings();

    DeviceConfig config;
    config.mode = static_cast<ModbusMode>(
        settings.value("connection/mode", static_cast<int>(ModbusMode::Tcp)).toInt()
    );
    config.tcp.host = settings.value("connection/tcpHost", "127.0.0.1").toString();
    config.tcp.port = settings.value("connection/tcpPort", 5020).toInt();
    config.serial.portName = settings.value("connection/serialPort", "COM13").toString();
    config.serial.baudRate = settings.value("connection/baudRate", 9600).toInt();
    config.slaveId = settings.value("connection/slaveId", 1).toInt();

    return config;
}

void AppSettings::saveDeviceConfig(const DeviceConfig &config) const
{
    QSettings settings = createSettings();

    settings.setValue("connection/mode", static_cast<int>(config.mode));
    settings.setValue("connection/tcpHost", config.tcp.host);
    settings.setValue("connection/tcpPort", config.tcp.port);
    settings.setValue("connection/serialPort", config.serial.portName);
    settings.setValue("connection/baudRate", config.serial.baudRate);
    settings.setValue("connection/slaveId", config.slaveId);
}

PollingConfig AppSettings::loadPollingConfig() const
{
    QSettings settings = createSettings();

    PollingConfig config;
    config.intervalMs = settings.value("polling/intervalMs", config.intervalMs).toInt();
    config.startAddress = settings.value("polling/startAddress", config.startAddress).toInt();
    config.count = settings.value("polling/count", config.count).toInt();

    return config;
}

void AppSettings::savePollingConfig(const PollingConfig &config) const
{
    QSettings settings = createSettings();

    settings.setValue("polling/intervalMs", config.intervalMs);
    settings.setValue("polling/startAddress", config.startAddress);
    settings.setValue("polling/count", config.count);
}

AlarmLimits AppSettings::loadAlarmLimits(double defaultTemperatureHighLimit,
                                         double defaultVoltageLowLimit) const
{
    // 阈值默认值来自 AlarmManager，避免配置层和业务层各自维护一套默认值。
    QSettings settings = createSettings();

    AlarmLimits limits;
    limits.temperatureHighLimit =
        settings.value("界限/最高温度", defaultTemperatureHighLimit).toDouble();
    limits.voltageLowLimit =
        settings.value("界限/最低电压", defaultVoltageLowLimit).toDouble();

    return limits;
}

void AppSettings::saveAlarmLimits(const AlarmLimits &limits) const
{
    QSettings settings = createSettings();

    settings.setValue("界限/最高温度", limits.temperatureHighLimit);
    settings.setValue("界限/最低电压", limits.voltageLowLimit);
}

ReconnectConfig AppSettings::loadReconnectConfig(bool defaultEnabled,
                                                 int defaultIntervalMs) const
{
    QSettings settings = createSettings();

    ReconnectConfig config;
    config.enabled = settings.value("reconnect/enabled", defaultEnabled).toBool();
    config.intervalMs = settings.value("reconnect/intervalMs", defaultIntervalMs).toInt();

    return config;
}

void AppSettings::saveReconnectConfig(const ReconnectConfig &config) const
{
    QSettings settings = createSettings();

    settings.setValue("reconnect/enabled", config.enabled);
    settings.setValue("reconnect/intervalMs", config.intervalMs);
}

QSettings AppSettings::createSettings() const
{
    return QSettings(kOrganization, kApplication);
}
