#include "TrendPanel.h"

#include <QHBoxLayout>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>

TrendPanel::TrendPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    setMinimumHeight(420); // 保证 4 行曲线的最小绘制空间，避免挤在一起
}

void TrendPanel::setupUi()
{
    pauseButton = new QPushButton("Pause", this);
    clearButton = new QPushButton("Clear", this);

    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(pauseButton);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addStretch();

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch(); // 弹性空间由 paintEvent 自由绘制，按钮只占顶部一行

    connect(pauseButton, &QPushButton::clicked, this, &TrendPanel::togglePaused);
    connect(clearButton, &QPushButton::clicked, this, &TrendPanel::clearData);
}

void TrendPanel::appendValue(const EngineeringValue &value)
{
    if (paused) {
        return; // 暂停时丢弃新数据，保持曲线静止
    }

    values.append(value);

    while (values.size() > maxPointCount) {
        values.removeFirst(); // FIFO 滑动窗口：丢弃最早的旧数据
    }

    update(); // 触发重绘
}

void TrendPanel::clearData()
{
    values.clear();
    update(); // 重绘 → 显示 "Waiting for data..."
}

void TrendPanel::togglePaused()
{
    paused = !paused;
    pauseButton->setText(paused ? "Resume" : "Pause"); // 按钮文字跟随状态切换
}

void TrendPanel::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true); // 折线平滑

    // 留白区域：上 58px（标题 + 按钮栏），其余三边各 24px
    QRect area = rect().adjusted(24, 58, -24, -24);

    if (values.size() < 2) {
        painter.setPen(QColor(100, 100, 100));
        painter.drawText(area, Qt::AlignCenter, "Waiting for data...");
        return; // 不足 2 点无法成线
    }

    const int gap = 10;                                  // 行间距
    const int rowHeight = (area.height() - gap * 3) / 4; // 3 个间距，4 等分高度

    for (int i = 0; i < 4; ++i) {
        QRect rowRect(
            area.left(),
            area.top() + i * (rowHeight + gap),
            area.width(),
            rowHeight
        );

        switch (i) {
        case 0:
            drawSeries(painter, rowRect, 0, QColor(220, 70, 70), "Temperature");
            break;
        case 1:
            drawSeries(painter, rowRect, 1, QColor(70, 120, 220), "Voltage");
            break;
        case 2:
            drawSeries(painter, rowRect, 2, QColor(60, 160, 100), "Current");
            break;
        case 3:
            drawSeries(painter, rowRect, 3, QColor(180, 120, 40), "Speed");
            break;
        }
    }
}

double TrendPanel::metricValue(const EngineeringValue &value, int metricIndex) const
{
    // 按索引分发：0=温度, 1=电压, 2=电流, 3=转速
    switch (metricIndex) {
    case 0: return value.temperature;
    case 1: return value.voltage;
    case 2: return value.current;
    case 3: return value.speed;
    default: return 0.0; // 防御：非法索引不会在正常路径触发
    }
}

void TrendPanel::drawSeries(QPainter &painter, const QRect &rect, int metricIndex,
                            const QColor &color, const QString &name)
{
    // 获取对应物理量单位
    QString unit;
    switch (metricIndex) {
    case 0: unit = "C"; break;   // 摄氏度
    case 1: unit = "V"; break;   // 伏特
    case 2: unit = "A"; break;   // 安培
    case 3: unit = "rpm"; break; // 转/分
    }

    // 第 1 步：遍历数据，计算 Y 轴范围
    double minValue = metricValue(values.first(), metricIndex);
    double maxValue = minValue;

    for (const auto &value : values) {
        const double current = metricValue(value, metricIndex);
        minValue = qMin(minValue, current);
        maxValue = qMax(maxValue, current);
    }

    // 第 2 步：添加 Y 轴边距，避免曲线贴顶贴底
    if (qFuzzyCompare(minValue, maxValue)) {
        minValue -= 1.0;          // 所有值相等时手工扩 ±1，防止除零
        maxValue += 1.0;
    } else {
        const double padding = (maxValue - minValue) * 0.1; // 上下各 10%
        minValue -= padding;
        maxValue += padding;
    }

    painter.save(); // 保存画笔状态，restore 后不影响后续行

    // 第 3 步：背景 + 边框
    painter.fillRect(rect, QColor(250, 250, 250)); // 极浅灰底色
    painter.setPen(QPen(QColor(215, 215, 215), 1));
    painter.drawRect(rect);

    // 第 4 步：3 条水平参考线（4 等分），辅助读数
    for (int i = 1; i < 4; ++i) {
        const int y = rect.top() + rect.height() * i / 4;
        painter.setPen(QPen(QColor(232, 232, 232), 1));
        painter.drawLine(rect.left(), y, rect.right(), y);
    }

    // 第 5 步：左上角标题（名称 + 最新值 + 单位）
    const double latest = metricValue(values.last(), metricIndex);

    painter.setPen(color);
    painter.drawText(
        rect.adjusted(10, 6, -10, -6),
        Qt::AlignLeft | Qt::AlignTop,
        QString("%1  %2 %3")
            .arg(name)
            .arg(latest, 0, 'f', metricIndex == 3 ? 0 : 2) // Speed 用整数，其余 2 位小数
            .arg(unit)
    );

    // 第 6 步：右上角 min/max 范围
    painter.setPen(QColor(100, 100, 100));
    painter.drawText(
        rect.adjusted(10, 6, -10, -6),
        Qt::AlignRight | Qt::AlignTop,
        QString("min %1 / max %2")
            .arg(minValue, 0, 'f', metricIndex == 3 ? 0 : 2)
            .arg(maxValue, 0, 'f', metricIndex == 3 ? 0 : 2)
    );

    // 第 7 步：绘线区域（扣除标题行 + 边距）
    QRect plotRect = rect.adjusted(12, 28, -12, -10);

    // 第 8 步：数据点 → 像素坐标映射
    QPolygonF points;
    for (int i = 0; i < values.size(); ++i) {
        const double xRatio = double(i) / double(values.size() - 1); // X: 等分宽度
        const double current = metricValue(values.at(i), metricIndex);
        const double yRatio = (current - minValue) / (maxValue - minValue); // Y: [0,1] 归一化

        const double x = plotRect.left() + xRatio * plotRect.width();
        // 屏幕 Y 轴向下 → bottom - 偏移 = 数值大在上
        const double y = plotRect.bottom() - yRatio * plotRect.height();

        points.append(QPointF(x, y));
    }

    // 第 9 步：裁剪 + 绘制折线
    painter.setClipRect(plotRect);   // 防止曲线超出绘图区域
    painter.setPen(QPen(color, 2));  // 2px 线宽
    painter.drawPolyline(points);    // 不闭合的连续折线

    painter.restore(); // 恢复画笔 → 不影响后续行
}
