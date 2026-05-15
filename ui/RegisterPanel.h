#ifndef REGISTERPANEL_H
#define REGISTERPANEL_H

#include "../core/RegisterValue.h"

#include <QWidget>

class QPushButton;
class QSpinBox;
class QTableWidget;
class QComboBox;
class QLineEdit;

class RegisterPanel : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterPanel(QWidget *parent = nullptr);

signals:
    void readHoldingRegistersRequested(int startAddress, int count); //读取
    void readInputRegistersRequested(int startAddress, int count);
    void readCoilsRequested(int startAddress, int count);
    void readDiscreteInputsRequested(int startAddress, int count);
    void writeSingleHoldingRegisterRequested(int address, quint16 value); //写入
    void writeSingleCoilRequested(int address, bool value);
    void writeMultipleHoldingRegistersRequested(int startAddress, const QVector<quint16> &values);
    void writeMultipleCoilsRequested(int startAddress, const QVector<bool> &values);

public slots:
    void displayHoldingRegisters(const RegisterReadResult &result); //当成功读取寄存器后，通知界面更新显示数据。
    void displayRegisters(const RegisterReadResult &result);

private:
    void setupUi();
    bool matchesPendingRead(const RegisterReadResult &result) const;

private:
    QSpinBox *readStartAddressSpin = nullptr;
    QSpinBox *readCountSpin = nullptr;
    QPushButton *readButton = nullptr;

    QSpinBox *writeAddressSpin = nullptr;
    QSpinBox *writeValueSpin = nullptr;
    QPushButton *writeButton = nullptr;

    QSpinBox *writeCoilAddressSpin = nullptr;
    QComboBox *writeCoilValueCombo = nullptr;
    QPushButton *writeCoilButton = nullptr;

    QTableWidget *resultTable = nullptr;

    QComboBox *readTypeCombo = nullptr;

    QSpinBox *writeMultiRegisterStartSpin = nullptr;
    QLineEdit *writeMultiRegisterValuesEdit = nullptr;
    QPushButton *writeMultiRegisterButton = nullptr;

    QSpinBox *writeMultiCoilStartSpin = nullptr;
    QLineEdit *writeMultiCoilValuesEdit = nullptr;
    QPushButton *writeMultiCoilButton = nullptr;    

    bool hasPendingRead = false;
    RegisterType pendingReadType = RegisterType::HoldingRegister;
    int pendingReadStartAddress = 0;
    int pendingReadCount = 0;
};


#endif // REGISTERPANEL_H
