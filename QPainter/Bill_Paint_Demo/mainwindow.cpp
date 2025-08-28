#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPrinter>
#include <QPrintDialog>
#include <QTextDocument>
#include <QDate>
#include <QTextTable>
#include <QTextCursor>

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

void MainWindow::printBill(const QString &customerName,
                           const QString &contact,
                           const QVector<QPair<QString, QPair<int, double>>> &products)
{
    // ====== Create printer ======
    QPrinter printer(QPrinter::HighResolution);
    // For testing → generate PDF instead of physical printer
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName("bill.pdf");

    // If you want real printer, use:
    // QPrintDialog dialog(&printer, this);
    // if (dialog.exec() != QDialog::Accepted)
    //     return;

    // ====== QTextDocument ======
    QTextDocument doc;
    QTextCursor cursor(&doc);

    // ====== HEADER ======
    QTextBlockFormat centerFormat;
    centerFormat.setAlignment(Qt::AlignCenter);

    QTextCharFormat boldFormat;
    boldFormat.setFontPointSize(14);
    boldFormat.setFontWeight(QFont::Bold);

    cursor.insertBlock(centerFormat, boldFormat);
    cursor.insertText("AEROWAVE SYSTEMS\n");

    cursor.insertBlock();

    // Bill No + Date
    cursor.insertHtml(QString("<table width='100%'><tr>"
                              "<td><b>Bill No:</b> %1</td>"
                              "<td align='right'><b>Date:</b> %2</td>"
                              "</tr></table>")
                          .arg("001")
                          .arg(QDate::currentDate().toString("dd-MM-yyyy")));

    cursor.insertBlock();
    cursor.insertText(QString("Customer: %1\nContact: %2\n\n")
                          .arg(customerName)
                          .arg(contact));

    // ====== PRODUCT TABLE ======
    QTextTableFormat tableFormat;
    tableFormat.setAlignment(Qt::AlignCenter);
    tableFormat.setBorder(1);
    tableFormat.setCellPadding(4);
    tableFormat.setCellSpacing(0);
    tableFormat.setHeaderRowCount(1);

    QTextTable *table = cursor.insertTable(products.size() + 2, 4, tableFormat);

    // Header row
    table->cellAt(0, 0).firstCursorPosition().insertText("S.No");
    table->cellAt(0, 1).firstCursorPosition().insertText("Product");
    table->cellAt(0, 2).firstCursorPosition().insertText("Qty");
    table->cellAt(0, 3).firstCursorPosition().insertText("Price");

    double total = 0;
    for (int i = 0; i < products.size(); ++i)
    {
        auto [productName, qtyPrice] = products[i];
        int qty = qtyPrice.first;
        double price = qtyPrice.second;

        table->cellAt(i + 1, 0).firstCursorPosition().insertText(QString::number(i + 1));
        table->cellAt(i + 1, 1).firstCursorPosition().insertText(productName);
        table->cellAt(i + 1, 2).firstCursorPosition().insertText(QString::number(qty));
        table->cellAt(i + 1, 3).firstCursorPosition().insertText(QString::number(price, 'f', 2));

        total += qty * price;
    }

    // Total row
    table->cellAt(products.size() + 1, 2).firstCursorPosition().insertText("Total");
    table->cellAt(products.size() + 1, 3).firstCursorPosition().insertText(QString::number(total, 'f', 2));

    // ====== FOOTER ======
    cursor.insertBlock();
    cursor.insertBlock();
    cursor.insertHtml("<p align='center'><b>Thank you for shopping with us!</b></p>");

    // ====== PRINT ======
    doc.print(&printer);
}



void MainWindow::on_pushButton_clicked()
        {
            QString customerName = "John Doe";
            QString contact = "9876543210";

            QVector<QPair<QString, QPair<int, double>>> products;
            products.append({ "Laptop", {1, 55000.00} });
            products.append({ "Mouse", {2, 750.00} });
            products.append({ "Keyboard", {1, 1250.50} });

            printBill(customerName, contact, products);
        }

