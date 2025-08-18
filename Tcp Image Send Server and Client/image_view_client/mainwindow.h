#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void tcpReady();
    void tcpError(QAbstractSocket::SocketError error);

private:
    Ui::MainWindow *ui;
    QTcpSocket m_socket;
    int m_datasize;
};
#endif // MAINWINDOW_H
