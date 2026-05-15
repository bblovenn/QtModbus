#ifndef RECONNECTCONFIGPANEL_H
#define RECONNECTCONFIGPANEL_H

#include <QWidget>

class QCheckBox;
class QSpinBox;
class QPushButton;

// 重连配置面板：控制自动重连开关和重连间隔。
class ReconnectConfigPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ReconnectConfigPanel(QWidget *parent = nullptr);
    void setInitialConfig(bool enabled, int intervalMs);

signals:
    void reconnectConfigChanged(bool enabled, int intervalMs);

private slots:
    void applyConfig();
    void resetToDefault();

private:
    void setupUi();

private:
    QCheckBox *enabledCheckBox = nullptr;
    QSpinBox *intervalSpinBox = nullptr;
    QPushButton *applyButton = nullptr;
    QPushButton *resetButton = nullptr;
};

#endif
