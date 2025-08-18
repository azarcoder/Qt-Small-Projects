#include "chartview.h"

ChartView::ChartView(QWidget *parent)
    : QChartView(parent)
{
    setRubberBand(QChartView::NoRubberBand);
}

ChartView::ChartView(QChart *chart, QWidget *parent)
    : QChartView(chart, parent)
{
    setRubberBand(QChartView::NoRubberBand);
}

void ChartView::enablePanning(bool enabled)
{
    m_panningEnabled = enabled;
    setCursor(enabled ? Qt::OpenHandCursor : Qt::ArrowCursor);
}

void ChartView::mousePressEvent(QMouseEvent *event)
{
    if (m_panningEnabled && event->button() == Qt::LeftButton) {
        m_isPanning = true;
        m_lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    QChartView::mousePressEvent(event);
}

void ChartView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_panningEnabled && m_isPanning) {
        QPointF delta = event->pos() - m_lastMousePos;
        chart()->scroll(-delta.x(), delta.y());
        m_lastMousePos = event->pos();
        event->accept();
        return;
    }
    QChartView::mouseMoveEvent(event);
}

void ChartView::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_panningEnabled && event->button() == Qt::LeftButton) {
        m_isPanning = false;
        setCursor(Qt::OpenHandCursor);
        event->accept();
        return;
    }
    QChartView::mouseReleaseEvent(event);
}
