#ifndef CONNECTIONPANEL_H
#define CONNECTIONPANEL_H

#include "../core/DeviceConfig.h"

#include <QWidget>

class QComboBox;
class QLineEdit;
class QPushButton;
class QSpinBox;

class ConnectionPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionPanel(QWidget *parent = nullptr);

signals:
    void connectRequested(const DeviceConfig &config);
    void disconnectRequested();

private:
    void setupUi();
    DeviceConfig currentConfig() const;

private:
    QComboBox *modeCombo = nullptr;
    QComboBox *serialPortCombo = nullptr;
    QComboBox *baudRateCombo = nullptr;
    QLineEdit *hostEdit = nullptr;
    QSpinBox *portSpin = nullptr;
    QSpinBox *slaveIdSpin = nullptr;
    QPushButton *connectButton = nullptr;
    QPushButton *disconnectButton = nullptr;

};


#endif // CONNECTIONPANEL_H
