#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTextStream>
#include <QTimer>
#include <QDebug>
#include <QMessageBox>

QSerialPort serial;
QTextStream cin(stdin);
QTextStream cout(stdout);

void MainWindow::readData()
{
    static QByteArray buffer;               // keep incomplete data
    buffer.append(serial.readAll());        // append new chunk

    int endIndex;
    while ((endIndex = buffer.indexOf('\n')) != -1)
    {
        QByteArray line = buffer.left(endIndex).trimmed(); // full line
        buffer.remove(0, endIndex + 1);   // remove processed part

        QString response = QString::fromUtf8(line);
        qDebug() << "Full Response:" << response;


        if (response.startsWith("TEMP:")) {
            QString value = response.section(":", 1, 1);
            ui->lineEdit_temp->setText(value + " °C");
        }
        else if (response.startsWith("LED:") || response.startsWith("LED")) {
            QString value = response.section(":", 1, 1);
            if (value == "ON")
            {
                ui->frame->setStyleSheet("background-color: green;");
            }
            else
            {
                ui->frame->setStyleSheet("background-color: red;");
            }
        }
        else if (response.startsWith("SENSOR1:")) {
            QString value = response.section(":", 1, 1);
            ui->lineEdit_sensor->setText(value);
        }
        else {
            QMessageBox::information(this, "info", "unknown : " + response);
        }
    }
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    serial.setPortName("COM7");
    serial.setBaudRate(QSerialPort::Baud9600);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);

    if (!serial.open(QIODevice::ReadWrite)) {
        qCritical() << "Failed to open serial port!";
        return;
    }

    connect(&serial, &QSerialPort::readyRead, this, &MainWindow::readData);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_checkBox_clicked(bool checked)
{
    if(checked)
        serial.write("SET_LED ON\n");
    else
        serial.write("SET_LED OFF\n");

    QTimer::singleShot(100, this, [=]{
        serial.write("GET_LED_STS\n");
    });
}


void MainWindow::on_pushButton_get_temp_clicked()
{
    serial.write("GET_TEMP\n");
}


void MainWindow::on_pushButton_get_sensor_clicked()
{
    serial.write("GET_SENSOR_1\n");
}

