#ifndef ALARMPANEL_H
#define ALARMPANEL_H

#include "../core/AlarmRecord.h"

#include <QWidget>
#include <QDateTime>
#include <QAbstractItemView>

class QPushButton;
class QTableWidget;

class AlarmPanel : public QWidget
{
    Q_OBJECT

public:
    explicit AlarmPanel(QWidget *parent = nullptr);

public slots:
    void appendAlarm(const AlarmRecord &alarm);
    void confirmSelectedAlarm();
    void clear();

signals:
    void alarmConfirmed(const QString &alarmId, const QDateTime &confirmedTime);

private:
    void setupUi();
    QString alarmTypeText(AlarmType type) const;
    QString alarmLevelText(AlarmLevel level) const;

private:
    QTableWidget *table = nullptr;
    QPushButton *confirmButton = nullptr;
    QPushButton *clearButton = nullptr;
};

#endif // ALARMPANEL_H
