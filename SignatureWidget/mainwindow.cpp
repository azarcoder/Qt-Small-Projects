#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    sig = new SignatureWidget(this);
    ui->horizontalLayout_signature->addWidget(sig);

}

MainWindow::~MainWindow()
{
    delete ui;
}




void MainWindow::on_pushButton_clr_clicked()
{
    sig->clear();
}


void MainWindow::on_pushButton_save_clicked()
{
    sig->getSignature().save("signature.png");

}

