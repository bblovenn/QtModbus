#include "../ui/RegisterPanel.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QMessageBox>

RegisterPanel::RegisterPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void RegisterPanel::setupUi()
{
    // 页面只收集用户输入并发信号，具体 Modbus 请求由通信层处理。
    // —— 读取参数：地址、数量、数据区类型 ——
    readStartAddressSpin = new QSpinBox(this);
    readStartAddressSpin->setRange(0, 65535);
    readStartAddressSpin->setValue(0);

    readCountSpin = new QSpinBox(this);
    readCountSpin->setRange(1, 125);
    readCountSpin->setValue(4);

    readTypeCombo = new QComboBox(this);
    readTypeCombo->addItem("保持寄存器 03", static_cast<int>(RegisterType::HoldingRegister));
    readTypeCombo->addItem("输入寄存器 04", static_cast<int>(RegisterType::InputRegister));
    readTypeCombo->addItem("线圈 01", static_cast<int>(RegisterType::Coil));
    readTypeCombo->addItem("离散输入 02", static_cast<int>(RegisterType::DiscreteInput));

    readButton = new QPushButton("读取", this);

    auto *readForm = new QFormLayout;
    readForm->addRow("数据区", readTypeCombo);
    readForm->addRow("起始地址", readStartAddressSpin);
    readForm->addRow("读取数量", readCountSpin);
    readForm->addRow(readButton);

    auto *readGroup = new QGroupBox("读取", this);
    readGroup->setLayout(readForm);

    writeAddressSpin = new QSpinBox(this);
    writeAddressSpin->setRange(0, 65535);
    writeAddressSpin->setValue(10);

    writeValueSpin = new QSpinBox(this);
    writeValueSpin->setRange(0, 65535);
    writeValueSpin->setValue(0);

    writeButton = new QPushButton("写单个保持寄存器", this);

    writeCoilAddressSpin = new QSpinBox(this);
    writeCoilAddressSpin->setRange(0, 65535);
    writeCoilAddressSpin->setValue(0);

    writeCoilValueCombo = new QComboBox(this);
    writeCoilValueCombo->addItem("OFF", false);
    writeCoilValueCombo->addItem("ON", true);

    writeCoilButton = new QPushButton("写单个线圈", this);

    auto *writeForm = new QFormLayout;
    writeForm->addRow("写入地址", writeAddressSpin);
    writeForm->addRow("写入值", writeValueSpin);
    writeForm->addRow(writeButton);
    writeForm->addRow("线圈地址", writeCoilAddressSpin);
    writeForm->addRow("线圈值", writeCoilValueCombo);
    writeForm->addRow(writeCoilButton);

    auto *writeGroup = new QGroupBox("写入", this);
    writeGroup->setLayout(writeForm);

    resultTable = new QTableWidget(this);
    resultTable->setColumnCount(3);
    resultTable->setHorizontalHeaderLabels({"地址", "原始值", "十六进制"});
    resultTable->horizontalHeader()->setStretchLastSection(true);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(readGroup);
    mainLayout->addWidget(writeGroup);
    mainLayout->addWidget(resultTable);

    // —— 批量写保持寄存器 ——
    writeMultiRegisterStartSpin = new QSpinBox(this);
    writeMultiRegisterStartSpin->setRange(0, 65535);
    writeMultiRegisterStartSpin->setValue(20);

    //值输入框
    writeMultiRegisterValuesEdit = new QLineEdit(this);
    writeMultiRegisterValuesEdit->setPlaceholderText("例如: 100,200,300");

    writeMultiRegisterButton = new QPushButton("写多个保持寄存器", this);

    // —— 批量写线圈 ——
    writeMultiCoilStartSpin = new QSpinBox(this);
    writeMultiCoilStartSpin->setRange(0, 65535);
    writeMultiCoilStartSpin->setValue(0);

    writeMultiCoilValuesEdit = new QLineEdit(this);
    writeMultiCoilValuesEdit->setPlaceholderText("例如: 1,0,1,1");

    writeMultiCoilButton = new QPushButton("写多个线圈", this);

    writeForm->addRow("批量寄存器起始地址", writeMultiRegisterStartSpin);
    writeForm->addRow("批量寄存器值", writeMultiRegisterValuesEdit);
    writeForm->addRow(writeMultiRegisterButton);

    writeForm->addRow("批量线圈起始地址", writeMultiCoilStartSpin);
    writeForm->addRow("批量线圈值", writeMultiCoilValuesEdit);
    writeForm->addRow(writeMultiCoilButton);

    connect(writeButton, &QPushButton::clicked, this, [this]() {
        emit writeSingleHoldingRegisterRequested(
            writeAddressSpin->value(),
            static_cast<quint16>(writeValueSpin->value())
        );
    }); 

    connect(writeCoilButton, &QPushButton::clicked, this, [this]() {
        emit writeSingleCoilRequested(
            writeCoilAddressSpin->value(),
            writeCoilValueCombo->currentData().toBool()
        );
    });

    connect(readButton, &QPushButton::clicked, this, [this]() {
        const auto type = static_cast<RegisterType>(readTypeCombo->currentData().toInt());
        const int startAddress = readStartAddressSpin->value();
        const int count = readCountSpin->value();

        // 记录"待处理读取"参数，用于在 displayRegisters 中过滤掉轮询等非用户触发的读结果
        hasPendingRead = true;
        pendingReadType = type;
        pendingReadStartAddress = startAddress;
        pendingReadCount = count;

        if (type == RegisterType::HoldingRegister) {
            emit readHoldingRegistersRequested(startAddress, count);
        } else if (type == RegisterType::InputRegister) {
            emit readInputRegistersRequested(startAddress, count);
        } else if (type == RegisterType::Coil) {
            emit readCoilsRequested(startAddress, count);
        } else {
            emit readDiscreteInputsRequested(startAddress, count);
        }
    });

    connect(writeMultiRegisterButton, &QPushButton::clicked, this, [this]() {
        QVector<quint16> values; //存放解析后的寄存器值
        //解析输入框中的文本，按逗号分隔，转换为整数，并验证范围
        const QStringList parts = writeMultiRegisterValuesEdit->text().split(',', Qt::SkipEmptyParts);

        for (const QString &part : parts) { //遍历每段
            bool ok = false;
            const int value = part.trimmed().toInt(&ok); //去除空格并转换为整数

            if (!ok || value < 0 || value > 65535) {
                QMessageBox::warning(this, "输入错误", "保持寄存器值必须是 0 到 65535 的整数，用逗号分隔。");
                return;
            }

            values.append(static_cast<quint16>(value));
        }

        if (values.isEmpty()) {
            QMessageBox::warning(this, "输入错误", "请输入至少一个保持寄存器值。");
            return;
        }

        emit writeMultipleHoldingRegistersRequested(
            writeMultiRegisterStartSpin->value(), //起始地址
            values
        );
    });

    //批量写线圈
    connect(writeMultiCoilButton, &QPushButton::clicked, this, [this]() {
        QVector<bool> values;
        const QStringList parts = writeMultiCoilValuesEdit->text().split(',', Qt::SkipEmptyParts);

        for (const QString &part : parts) {
            const QString text = part.trimmed().toLower();

            if (text == "1" || text == "true" || text == "on") {
                values.append(true);
            } else if (text == "0" || text == "false" || text == "off") {
                values.append(false);
            } else {
                QMessageBox::warning(this, "输入错误", "线圈值只能是 1/0、true/false 或 on/off，用逗号分隔。");
                return;
            }
        }

        if (values.isEmpty()) {
            QMessageBox::warning(this, "输入错误", "请输入至少一个线圈值。");
            return;
        }

        emit writeMultipleCoilsRequested(
            writeMultiCoilStartSpin->value(),
            values
        );
    }); 
}

// 显示读取结果到表格
void RegisterPanel::displayHoldingRegisters(const RegisterReadResult &result)
{
    // 设置表格行数
    resultTable->setRowCount(result.values.size());

    // 填充表格数据
    for (int row = 0; row < result.values.size(); ++row) {
        const RegisterValue value = result.values.at(row);

        // 寄存器地址
        resultTable->setItem(row, 0, new QTableWidgetItem(QString::number(value.address)));
        // 原始十进制寄存器值
        resultTable->setItem(row, 1, new QTableWidgetItem(QString::number(value.rawValue)));
        // 十六进制寄存器值
        resultTable->setItem(row, 2, new QTableWidgetItem(
            QString("0x%1").arg(value.rawValue, 4, 16, QLatin1Char('0')).toUpper()
        ));
    }
}

void RegisterPanel::displayRegisters(const RegisterReadResult &result)
{
    // 过滤：只显示用户手动发起的读取结果，忽略轮询等自动读取
    if (!matchesPendingRead(result)) {
        return;
    }

    RegisterReadResult displayResult = result;
    if (displayResult.values.size() > pendingReadCount) {
        //如果返回的值比请求的数量多，截取前 pendingReadCount 个值进行显示，避免界面显示过多数据。
        displayResult.values = displayResult.values.mid(0, pendingReadCount); 
    }

    //重置 pendingRead 状态，准备下一次读取请求的匹配
    hasPendingRead = false;
    displayHoldingRegisters(displayResult);
}

bool RegisterPanel::matchesPendingRead(const RegisterReadResult &result) const
{
    return hasPendingRead
        && result.type == pendingReadType
        && result.startAddress == pendingReadStartAddress
        && result.values.size() >= pendingReadCount;
}
