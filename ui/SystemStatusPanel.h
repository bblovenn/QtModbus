#ifndef SYSTEMSTATUSPANEL_H
#define SYSTEMSTATUSPANEL_H

#include "../core/AlarmRecord.h"
#include "../core/EngineeringValue.h"

#include <QWidget>

class QLabel;

class SystemStatusPanel : public QWidget
{
    Q_OBJECT

public:
    explicit SystemStatusPanel(QWidget *parent = nullptr);

public slots:
    void setConnecting();
    void setConnected();
    void setDisconnected();
    void setCommunicationError(const QString &message);
    void setPollingStarted();
    void setPollingStopped();
    void updateEngineeringValue(const EngineeringValue &value);
    void updateAlarm(const AlarmRecord &alarm);
    void setDatabaseReady(const QString &path);

private:
    void setupUi();
    void setLabelText(QLabel *label, const QString &title, const QString &value);

private:
    QLabel *connectionLabel = nullptr;
    QLabel *pollingLabel = nullptr;
    QLabel *databaseLabel = nullptr;
    QLabel *lastUpdateLabel = nullptr;
    QLabel *temperatureLabel = nullptr;
    QLabel *voltageLabel = nullptr;
    QLabel *currentLabel = nullptr;
    QLabel *speedLabel = nullptr;
    QLabel *alarmLabel = nullptr;
};

#endif
