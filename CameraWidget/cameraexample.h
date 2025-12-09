#ifndef CAMERAEXAMPLE_H
#define CAMERAEXAMPLE_H

#include <QWidget>
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QLabel>
#include <QPushButton>

class CameraExample : public QWidget
{
    Q_OBJECT

public:
    explicit CameraExample(QWidget *parent = nullptr);

private slots:
    void captureImage();
    void imageSaved(int id, const QString &fileName);
    void imageCaptured(int id, const QImage &preview);

private:
    QCamera *camera;
    QCameraViewfinder *viewfinder;
    QCameraImageCapture *imageCapture;

    QPushButton *captureButton;
    QLabel *previewLabel;
};

#endif // CAMERAEXAMPLE_H
