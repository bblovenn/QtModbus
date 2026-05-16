#include "PacketMonitorPanel.h"

#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

PacketMonitorPanel::PacketMonitorPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void PacketMonitorPanel::setupUi()
{
    logEdit = new QTextEdit(this);
    logEdit->setReadOnly(true);

    clearButton = new QPushButton("清空日志", this);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(logEdit);
    mainLayout->addWidget(clearButton);

    connect(clearButton, &QPushButton::clicked,
            this, &PacketMonitorPanel::clear);
}

void PacketMonitorPanel::appendPacket(const PacketRecord &record)
{
    logEdit->append(formatPacket(record));
}

void PacketMonitorPanel::appendText(const QString &text)
{
    logEdit->append(text);
}

void PacketMonitorPanel::clear()
{
    logEdit->clear();
}

//将报文记录格式化为可读字符串
QString PacketMonitorPanel::formatPacket(const PacketRecord &record) const
{
    const QString directionText =
        // 根据方向生成标签：发送=TX，接收=RX
        record.direction == PacketDirection::Tx ? "发送" : "接收";

    // 将字节数组转为十六进制字符串，每字节用空格分隔，转大写
    // 例如：QByteArray{0x01,0x03} → "01 03"
    const QString hexText =
        QString(record.data.toHex(' ').toUpper());

    // 组合成最终字符串
    // 格式：[时间] TX/RX: 十六进制数据 描述
    return QString("[%1] %2: %3 %4")
        .arg(record.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz"))
        .arg(directionText)
        .arg(hexText)
        .arg(record.description);
}
