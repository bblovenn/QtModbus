#ifndef POLLINGCONFIGPANEL_H
#define POLLINGCONFIGPANEL_H

#include <QWidget>

class QSpinBox;
class QPushButton;

// 采集配置面板：维护轮询周期、起始地址和读取数量。
class PollingConfigPanel : public QWidget
{
    Q_OBJECT

public:
    explicit PollingConfigPanel(QWidget *parent = nullptr);

    //外部加载初始配置，通常来自持久化存储（如配置文件或数据库）
    void setInitialConfig(int intervalMs, int startAddress, int count);

signals:
    void pollingConfigChanged(int intervalMs, int startAddress, int count);

private slots:
    void applyConfig();
    //重置配置为默认值，通常是预定义的安全值
    void resetToDefault();

private:
    void setupUi();

private:
    QSpinBox *intervalSpin = nullptr;
    QSpinBox *startAddressSpin = nullptr;
    QSpinBox *countSpin = nullptr;
    QPushButton *applyButton = nullptr;
    QPushButton *resetButton = nullptr;
};

#endif
