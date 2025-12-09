#ifndef SIGNATUREWIDGET_H
#define SIGNATUREWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QPoint>

class SignatureWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SignatureWidget(QWidget *parent = nullptr);

    void clear();                 // Clear signature
    QPixmap getSignature() const; // Return signature image

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QPixmap pixmap;
    QPoint lastPos;

    void initPixmap(); // internal helper to resize pixmap cleanly
};

#endif // SIGNATUREWIDGET_H
