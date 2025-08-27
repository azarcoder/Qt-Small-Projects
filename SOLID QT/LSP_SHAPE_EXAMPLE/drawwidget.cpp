#include "drawwidget.h"

DrawWidget::DrawWidget(QWidget *parent) : QWidget(parent)
{

}

void DrawWidget::setShape(Shape *shape)
{
    currentShape = shape;
    update();   // triggers paintEvent
}

void DrawWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    if (currentShape) {
        currentShape->draw(&painter);
    }
}
