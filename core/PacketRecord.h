#ifndef PACKETRECORD_H
#define PACKETRECORD_H

#include <QByteArray>
#include <QDateTime>
#include <QMetaType>
#include <QString>

// 报文方向：Tx 表示上位机发出，Rx 表示设备响应。
enum class PacketDirection {
    Tx,
    Rx
};

// 通信报文日志记录，用于联调时追踪请求、响应和错误摘要。
struct PacketRecord
{
    QDateTime timestamp = QDateTime::currentDateTime();;
    PacketDirection direction = PacketDirection::Tx;
    QByteArray data;
    QString description; //描述信息
};

Q_DECLARE_METATYPE(PacketRecord)

#endif // PACKETRECORD_H
