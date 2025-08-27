#ifndef SHAPE_H
#define SHAPE_H
#include <QPainter>
class Shape {
public:
    virtual void draw(QPainter* painter) = 0;
    virtual ~Shape() {}
};

#endif // SHAPE_H
