#ifndef PACKETMONITORPANEL_H
#define PACKETMONITORPANEL_H

#include "../core/PacketRecord.h"

#include <QWidget>

class QPushButton;
class QTextEdit;

class PacketMonitorPanel : public QWidget
{
    Q_OBJECT

public:
    explicit PacketMonitorPanel(QWidget *parent = nullptr);

public slots:
    void appendPacket(const PacketRecord &record); // 添加一条报文记录
    void appendText(const QString &text);
    void clear();

private:
    void setupUi();
    QString formatPacket(const PacketRecord &record) const;// 格式化报文

private:
    QTextEdit *logEdit = nullptr;
    QPushButton *clearButton = nullptr;
};

#endif // PACKETMONITORPANEL_H
