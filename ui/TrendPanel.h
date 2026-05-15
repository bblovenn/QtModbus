#ifndef TRENDPANEL_H
#define TRENDPANEL_H

#include "../core/EngineeringValue.h"

#include <QColor>
#include <QRect>
#include <QString>
#include <QWidget>
#include <QVector>

class QPainter;
class QPushButton;

// 实时曲线面板：绘制采集数据随时间变化的趋势。
class TrendPanel : public QWidget
{
    Q_OBJECT

public:
    explicit TrendPanel(QWidget *parent = nullptr);

public slots:
    void appendValue(const EngineeringValue &value);
    void setValues(const QVector<EngineeringValue> &newValues);
    void setControlPanelVisible(bool visible);

private slots:
    void clearData();
    void togglePaused();

protected:
    // 重写 QWidget 的 paintEvent，使用 QPainter 手绘坐标轴、网格、曲线
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUi();
    // 绘制单条曲线（如温度曲线、压力曲线），metricIndex 指定从 EngineeringValue 取哪个字段
    void drawSeries(QPainter &painter, const QRect &rect, int metricIndex,
                    const QColor &color, const QString &name);
    // 从 EngineeringValue 中按索引提取数值（0=温度, 1=压力, …），多曲线共用同一组数据点
    double metricValue(const EngineeringValue &value, int metricIndex) const;

private:
    QVector<EngineeringValue> values;
    int maxPointCount = 120;
    bool paused = false;

    QPushButton *pauseButton = nullptr;
    QPushButton *clearButton = nullptr;
};

#endif
