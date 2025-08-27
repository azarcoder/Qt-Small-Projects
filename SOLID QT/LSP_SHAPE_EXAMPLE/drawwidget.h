#ifndef DRAWWIDGET_H
#define DRAWWIDGET_H

#include <QWidget>
#include "Shape.h"

class DrawWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DrawWidget(QWidget *parent = nullptr);
    void setShape(Shape* shape);

signals:


    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event);
private:
    Shape* currentShape = nullptr;
};

#endif // DRAWWIDGET_H
