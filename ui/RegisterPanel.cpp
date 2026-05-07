#include "../ui/RegisterPanel.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

RegisterPanel::RegisterPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void RegisterPanel::setupUi()
{
    readStartAddressSpin = new QSpinBox(this);
    readStartAddressSpin->setRange(0, 65535);
    readStartAddressSpin->setValue(0);

    readCountSpin = new QSpinBox(this);
    readCountSpin->setRange(1, 125);
    readCountSpin->setValue(4);

    readButton = new QPushButton("读取保持寄存器", this);

    auto *readForm = new QFormLayout;
    readForm->addRow("起始地址", readStartAddressSpin);
    readForm->addRow("读取数量", readCountSpin);
    readForm->addRow(readButton);

    auto *readGroup = new QGroupBox("读取", this);
    readGroup->setLayout(readForm);

    writeAddressSpin = new QSpinBox(this);
    writeAddressSpin->setRange(0, 65535);
    writeAddressSpin->setValue(0);

    writeValueSpin = new QSpinBox(this);
    writeValueSpin->setRange(0, 65535);
    writeValueSpin->setValue(0);

    writeButton = new QPushButton("写单个保持寄存器", this);

    auto *writeForm = new QFormLayout;
    writeForm->addRow("写入地址", writeAddressSpin);
    writeForm->addRow("写入值", writeValueSpin);
    writeForm->addRow(writeButton);

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

    connect(readButton, &QPushButton::clicked, this, [this]() {
        emit readHoldingRegistersRequested(
            readStartAddressSpin->value(),
            readCountSpin->value()
        );
    });

    connect(writeButton, &QPushButton::clicked, this, [this]() {
        emit writeSingleHoldingRegisterRequested(
            writeAddressSpin->value(),
            static_cast<quint16>(writeValueSpin->value())
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
