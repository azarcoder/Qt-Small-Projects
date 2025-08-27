#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "Shape.h"

class Triangle : public Shape
{
public:
    Triangle();
    void draw(QPainter* painter) override {
        QPolygon triangle;
        // Define 3 points of the triangle
        triangle << QPoint(100, 50)   // top vertex
                 << QPoint(50, 150)   // bottom-left
                 << QPoint(150, 150); // bottom-right

        painter->drawPolygon(triangle);
    }
};

#endif // TRIANGLE_H
