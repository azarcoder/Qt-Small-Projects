#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTextEdit>

#include <DockManager.h>
#include <DockWidget.h>

//documentations
//https://githubuser0xffff.github.io/Qt-Advanced-Docking-System/doc/user-guide.html

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // --- Create ADS CDockManager ---
    auto *CDockManager = new ads::CDockManager(this);

    // --- Example Dock Widgets ---
    QTextEdit *editor1 = new QTextEdit();
    editor1->setText("This is Dock 1");
    auto *dock1 = new ads::CDockWidget("Dock 1");
    dock1->setWidget(editor1);
    CDockManager->addDockWidget(ads::TopDockWidgetArea, dock1);

    QTextEdit *editor2 = new QTextEdit();
    editor2->setText("This is Dock 2");
    auto *dock2 = new ads::CDockWidget("Dock 2");
    dock2->setWidget(editor2);
    CDockManager->addDockWidget(ads::RightDockWidgetArea, dock2);

    QTextEdit *editor3 = new QTextEdit();
    editor3->setText("This is Dock 3");
    auto *dock3 = new ads::CDockWidget("Dock 3");
    dock3->setWidget(editor3);
    CDockManager->addDockWidget(ads::BottomDockWidgetArea, dock3);

    // --- Set CDockManager as central widget ---
    setCentralWidget(CDockManager);
}

MainWindow::~MainWindow()
{
    delete ui;
}
