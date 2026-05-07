#ifndef REGISTERPANEL_H
#define REGISTERPANEL_H

#include "../core/RegisterValue.h"

#include <QWidget>

class QPushButton;
class QSpinBox;
class QTableWidget;

class RegisterPanel : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterPanel(QWidget *parent = nullptr);

signals:
    void readHoldingRegistersRequested(int startAddress, int count); //读取
    void writeSingleHoldingRegisterRequested(int address, quint16 value); //写入

public slots:
    void displayHoldingRegisters(const RegisterReadResult &result); //当成功读取寄存器后，通知界面更新显示数据。

private:
    void setupUi();

private:
    QSpinBox *readStartAddressSpin = nullptr;
    QSpinBox *readCountSpin = nullptr;
    QPushButton *readButton = nullptr;

    QSpinBox *writeAddressSpin = nullptr;
    QSpinBox *writeValueSpin = nullptr;
    QPushButton *writeButton = nullptr;

    QTableWidget *resultTable = nullptr;
};


#endif // REGISTERPANEL_H