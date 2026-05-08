#ifndef ENGINEERINGVALUE_H
#define ENGINEERINGVALUE_H

#include <QDateTime>
#include <QMetaType>
#include <QString>

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
