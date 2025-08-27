#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Shape.h"
#include "circle.h"
#include "rectangle.h"
#include "triangle.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->comboBox->addItems({"Circle", "Rectangle", "Triangle"});
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    QString choice = ui->comboBox->currentText();
    Shape* shape = nullptr;

    if (choice == "Circle") shape = new Circle();
    else if (choice == "Rectangle") shape = new Rectangle();
    else if (choice == "Triangle") shape = new Triangle();

    ui->widget->setShape(shape);
}

