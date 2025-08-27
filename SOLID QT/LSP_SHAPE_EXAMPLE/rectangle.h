#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "Shape.h"

class Rectangle : public Shape
{
public:
    Rectangle();
    void draw(QPainter* painter) override {
        // Draw a rectangle at position (50,50), width=120, height=80
        painter->drawRect(50, 50, 120, 80);
    }
};

#endif // RECTANGLE_H
