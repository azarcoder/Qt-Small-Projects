#include "cameraexample.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QDir>
#include <QFile>

CameraExample::CameraExample(QWidget *parent)
    : QWidget(parent)
{
    // UI Setup
    viewfinder = new QCameraViewfinder(this);
    viewfinder->setMinimumSize(300, 200);

    captureButton = new QPushButton("Capture", this);

    previewLabel = new QLabel(this);
    previewLabel->setFixedSize(200, 200);
    previewLabel->setStyleSheet("border: 1px solid gray");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(viewfinder);
    layout->addWidget(captureButton);
    layout->addWidget(previewLabel);
    setLayout(layout);

    // Camera Setup
    camera = new QCamera(QCamera::availableDevices().value(0), this);
    camera->setViewfinder(viewfinder);

    imageCapture = new QCameraImageCapture(camera, this);

    camera->start();  // Start preview

    // Connections
    connect(captureButton, &QPushButton::clicked, this, &CameraExample::captureImage);
    connect(imageCapture, SIGNAL(imageCaptured(int,QImage)),
            this, SLOT(imageCaptured(int,QImage)));
    connect(imageCapture, SIGNAL(imageSaved(int,QString)),
            this, SLOT(imageSaved(int,QString)));
}

void CameraExample::captureImage()
{
    imageCapture->capture("captured_image.jpg");
}

void CameraExample::imageCaptured(int id, const QImage &preview)
{
    Q_UNUSED(id);
    previewLabel->setPixmap(QPixmap::fromImage(preview)
                            .scaled(200, 200, Qt::KeepAspectRatio));
}

void CameraExample::imageSaved(int id, const QString &fileName)
{
    Q_UNUSED(id);

    // Create folder if not exists
    QString folder = QDir::currentPath() + "/member_photos";
    QDir dir;

    if (!dir.exists(folder)) {
        dir.mkpath(folder);   // create folders recursively
    }

    // Build final file path (copy saved image to folder)
    QString newFilePath = folder + "/captured_image.jpg";

    // Move or copy the image to desired location
    if (QFile::exists(fileName)) {
        QFile::remove(newFilePath);  // remove old one if exists
        QFile::copy(fileName, newFilePath);
    }

    qDebug() << "Image saved to:" << newFilePath;
}

