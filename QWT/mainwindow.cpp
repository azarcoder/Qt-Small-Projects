#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
//    ui->myRadarKnob->setMarkerStyle(QwtKnob::Dot);
//    ui->myRadarKnob->setBorderWidth(5);
//    ui->myRadarKnob->setTotalAngle(360.0);
//    connect(ui->myRadarKnob, SIGNAL(valueChanged(double)),
//                this, SLOT(onKnobMoved(double)));

}

MainWindow::~MainWindow()
{
    delete ui;
}

