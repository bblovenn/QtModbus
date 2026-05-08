#ifndef PACKETRECORD_H
#define PACKETRECORD_H

#include <QByteArray>
#include <QDateTime>
#include <QMetaType>
#include <QString>

enum class PacketDirection {
    Tx,
    Rx
};

struct PacketRecord
{
    QDateTime timestamp = QDateTime::currentDateTime();;
    PacketDirection direction = PacketDirection::Tx;
    QByteArray data;
    QString description; //描述信息
};

Q_DECLARE_METATYPE(PacketRecord)

#endif // PACKETRECORD_H
