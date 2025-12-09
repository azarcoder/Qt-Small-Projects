#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cameraexample.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    CameraExample *cam = new CameraExample();
    cam->show();
}

