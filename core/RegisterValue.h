#ifndef REGISTERVALUE_H
#define REGISTERVALUE_H

#include <QDateTime>
#include <QMetaType>
#include <QString>
#include <QVector>

enum class RegisterType
{
    HoldingRegister, //保持寄存器,存储可读写的数据
    InputRegister, // 输入寄存器,存储只读数据
    Coil, //线圈,读写单个位
    DiscreteInput //离散输入,只读单个位
};

struct RegisterValue
{
    int address = 0; //寄存器地址
    quint16 rawValue = 0; //原始寄存器值
};

struct RegisterReadResult
{
    QString deviceId;
    RegisterType type = RegisterType::HoldingRegister; 
    int startAddress = 0; 
    QVector<RegisterValue> values; 
    QDateTime timestamp = QDateTime::currentDateTime(); //时间戳
};

struct RegisterWriteResult
{
    QString deviceId;
    RegisterType type = RegisterType::HoldingRegister;
    int address = 0;
    quint16 value = 0;
    int count = 0; //对于批量写入，表示写入的寄存器数量
    QDateTime timestamp = QDateTime::currentDateTime();
};

//注册为Qt元类型，以便在信号和槽,多线程中使用这些结构体。
Q_DECLARE_METATYPE(RegisterWriteResult)

Q_DECLARE_METATYPE(RegisterReadResult)

#endif // REGISTERVALUE_H
