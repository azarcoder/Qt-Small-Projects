#ifndef SERIALCORE_H
#define SERIALCORE_H

#include <QObject>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QTimer>
#include <QMessageBox>

class SerialCore : public QObject
{
    Q_OBJECT
public:
    explicit SerialCore(QObject *parent = nullptr);

    bool                m_bIsConnected = false;
    QSerialPort         *m_serialPort = nullptr;
    QByteArray           m_rxBuffer;

    int connect_serial(QString in_strPortName);
    bool disconnet_serial();
    int send_cmd(QByteArray byteArr);

public slots:
    void slot_read_data();

signals:
    void signal_parse_rx_buffer(const QByteArray &in_RxBuffer);

private:
    void create_objects();
};

#endif // SERIALCORE_H
