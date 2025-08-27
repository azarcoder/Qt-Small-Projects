#ifndef CIRCLE_H
#define CIRCLE_H

#include "Shape.h"

class Circle : public Shape
{
public:
    Circle();

    void draw(QPainter* painter) override {
        painter->drawEllipse(50, 50, 100, 100);
    }
};

#endif // CIRCLE_H
