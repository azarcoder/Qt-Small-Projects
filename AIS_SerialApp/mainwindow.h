#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "serialcore.h"
#include <QTimer>

extern "C" {
#include "ais_decode.h"
}

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    SerialCore                         *mObj_SerialCommunication;

    void set_serial_link_sts(bool sts);

private slots:
    void on_pushButton_uart_open_clicked();

    void on_pushButton_uart_close_clicked();

    void slot_parse_rx_buffer(const QByteArray &rx_byteArr);


private:
    Ui::MainWindow *ui;
    void init_gui();
    void connect_signals();

    //serial
    QTimer *m_NewDeviceTimer;
    void load_available_com_ports_into_comboBox();
    void enable_disable_uart_connection_button(bool sts);

    //debug purpose
    QString format_ais_type123(const S_AIS_TYPE123_FULL *in_pS);
    QString format_ais_type18(const S_AIS_TYPE18_FULL *in_pS);
};
#endif // MAINWINDOW_H
