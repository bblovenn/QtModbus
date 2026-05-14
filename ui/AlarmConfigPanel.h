#ifndef ALARMCONFIGPANEL_H
#define ALARMCONFIGPANEL_H

#include <QWidget>

class QDoubleSpinBox;
class QPushButton;

class AlarmConfigPanel : public QWidget
{
    Q_OBJECT

public:
    explicit AlarmConfigPanel(QWidget *parent = nullptr);

    void setInitialLimits(double temperatureHighLimit, double voltageLowLimit);

signals:
    //通知报警阈值已修改
    void alarmLimitsChanged(double temperatureHighLimit, double voltageLowLimit);

private slots:
    void applyLimits();
    void resetToDefault();

private:
    void setupUi();

private:
    QDoubleSpinBox *temperatureHighSpin = nullptr;
    QDoubleSpinBox *voltageLowSpin = nullptr;
    QPushButton *applyButton = nullptr;
    QPushButton *resetButton = nullptr;
};

#endif // ALARMCONFIGPANEL_H
