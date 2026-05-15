#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include "../core/DeviceConfig.h"

#include <QString>

// 周期采集配置，对应采集配置页中的三个输入项。
struct PollingConfig
{
    int intervalMs = 1000;
    int startAddress = 0;
    int count = 4;
};

// 报警阈值配置，目前用于温度上限和电压下限。
struct AlarmLimits
{
    double temperatureHighLimit = 0.0;
    double voltageLowLimit = 0.0;
};

// 自动重连配置，控制是否启用和重试间隔。
struct ReconnectConfig
{
    bool enabled = true;
    int intervalMs = 3000;
};

class QSettings;

// 应用配置读写入口。集中封装 QSettings 键名，避免配置散落在各个 UI/业务类里。
class AppSettings
{
public:
    AppSettings();

    DeviceConfig loadDeviceConfig() const;
    void saveDeviceConfig(const DeviceConfig &config) const;

    PollingConfig loadPollingConfig() const;
    void savePollingConfig(const PollingConfig &config) const;

    AlarmLimits loadAlarmLimits(double defaultTemperatureHighLimit,
                                double defaultVoltageLowLimit) const;
    void saveAlarmLimits(const AlarmLimits &limits) const;

    ReconnectConfig loadReconnectConfig(bool defaultEnabled,
                                        int defaultIntervalMs) const;
    void saveReconnectConfig(const ReconnectConfig &config) const;

private:
    QSettings createSettings() const;
};

#endif // APPSETTINGS_H
