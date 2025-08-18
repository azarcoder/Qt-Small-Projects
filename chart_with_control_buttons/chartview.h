#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QtCharts/QChartView>
#include <QMouseEvent>
#include <QPoint>

QT_CHARTS_USE_NAMESPACE

class ChartView : public QChartView
{
    Q_OBJECT

public:
    explicit ChartView(QWidget *parent = nullptr);
    explicit ChartView(QChart *chart, QWidget *parent = nullptr);

    void enablePanning(bool enabled);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    bool m_panningEnabled = false;
    bool m_isPanning = false;
    QPoint m_lastMousePos;
};

#endif // CHARTVIEW_H
