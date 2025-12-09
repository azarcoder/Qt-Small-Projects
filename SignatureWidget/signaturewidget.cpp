#include "signaturewidget.h"
#include <QPainter>
#include <QMouseEvent>

SignatureWidget::SignatureWidget(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StaticContents);
    initPixmap();
}

// ------------------------------------------------------------------
// Initialize pixmap with widget size
// ------------------------------------------------------------------
void SignatureWidget::initPixmap()
{
    pixmap = QPixmap(size());
    pixmap.fill(Qt::white);
}

// ------------------------------------------------------------------
// Clear drawing
// ------------------------------------------------------------------
void SignatureWidget::clear()
{
    pixmap.fill(Qt::white);
    update();
}

// ------------------------------------------------------------------
// Return signature as pixmap
// ------------------------------------------------------------------
QPixmap SignatureWidget::getSignature() const
{
    return pixmap;
}

// ------------------------------------------------------------------
// Mouse press → remember point
// ------------------------------------------------------------------
void SignatureWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

// ------------------------------------------------------------------
// Mouse move → draw on pixmap
// ------------------------------------------------------------------
void SignatureWidget::mouseMoveEvent(QMouseEvent *event)
{
    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::black, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawLine(lastPos, event->pos());

    lastPos = event->pos();
    update();
}

// ------------------------------------------------------------------
// PaintEvent → paint pixmap on widget
// ------------------------------------------------------------------
void SignatureWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, pixmap);
}

// ------------------------------------------------------------------
// Resize pixmap properly (old content preserved)
// ------------------------------------------------------------------
void SignatureWidget::resizeEvent(QResizeEvent *event)
{
    if (width() > pixmap.width() || height() > pixmap.height()) {

        QPixmap newPixmap(size());
        newPixmap.fill(Qt::white);

        QPainter painter(&newPixmap);
        painter.drawPixmap(0, 0, pixmap);

        pixmap = newPixmap;
    }

    QWidget::resizeEvent(event);
}
