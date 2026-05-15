#ifndef ENGINEERINGVALUE_H
#define ENGINEERINGVALUE_H

#include <QDateTime>
#include <QMetaType>
#include <QString>

// 轮询保持寄存器后换算得到的工程量，是监控、趋势和报警判断的共同输入。
struct EngineeringValue
{
    QString deviceId;
    QDateTime timestamp = QDateTime::currentDateTime();
    
    double temperature = 0.0;
    double voltage = 0.0;
    double current = 0.0; //电流
    double speed = 0.0;
};

Q_DECLARE_METATYPE(EngineeringValue)

#endif // ENGINEERINGVALUE_H
