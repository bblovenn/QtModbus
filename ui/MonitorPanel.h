#ifndef MONITORPANEL_H
#define MONITORPANEL_H

#include "../core/EngineeringValue.h"

#include <QWidget>

class QLabel;
class QPushButton;
class QTableWidget;

// 实时监控面板：展示最新工程量及最小值、最大值、平均值等统计信息。
class MonitorPanel : public QWidget
{
    Q_OBJECT

public:
    explicit MonitorPanel(QWidget *parent = nullptr);

public slots:
    void updateValue(const EngineeringValue &value);
    void clear();
    void togglePaused();  // 切换暂停/继续状态

private:
    void setupUi();
    void resetStats();
    void updateStats(double temperature, double voltage, double current, double speed);
    void updateTable();

private:
    QLabel *timestampLabel = nullptr;
    QTableWidget *table = nullptr;
    QPushButton *pauseButton = nullptr;
    QPushButton *clearButton = nullptr;

    bool paused = false;
    int sampleCount = 0;

    double currentTemperature = 0.0;
    double currentVoltage = 0.0;
    double currentCurrent = 0.0;
    double currentSpeed = 0.0;

    double minTemperature = 0.0;
    double minVoltage = 0.0;
    double minCurrent = 0.0;
    double minSpeed = 0.0;

    double maxTemperature = 0.0;
    double maxVoltage = 0.0;
    double maxCurrent = 0.0;
    double maxSpeed = 0.0;

    // 累加和（用于计算平均值）
    double sumTemperature = 0.0;
    double sumVoltage = 0.0;
    double sumCurrent = 0.0;
    double sumSpeed = 0.0;
};

#endif // MONITORPANEL_H
