#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "txtexporter.h"
#include "csvexporter.h"
#include "jsonexporter.h"
#include "brokenexporter.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Add items to combo box

    ui->comboBox->addItems({"TXT", "CSV", "JSON", "Broken"});
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_export_clicked()
{
    QString data = ui->textEdit->toPlainText();
    QString choice = ui->comboBox->currentText();

    Exporter* exporter = nullptr;

    if (choice == "TXT") exporter = new TxtExporter();
    else if (choice == "CSV") exporter = new CsvExporter();
    else if (choice == "JSON") exporter = new JsonExporter();
    else if (choice == "Broken") exporter = new BrokenExporter();

    bool success = false;
    if (exporter) {
        success = exporter->exportData(data, "output");
        delete exporter;
    }

    if (success) {
        ui->label_sts->setText("✅ Export successful!");
    } else {
        ui->label_sts->setText("❌ Export failed!");
    }
}

