#include "serialcore.h"
#include "mainwindow.h"
extern MainWindow *g_pMainwindow;
//#define DEBUG_PRINT_ENABLE
SerialCore::SerialCore(QObject *parent) : QObject(parent)
{
    create_objects();
}

int SerialCore::connect_serial(QString in_strPortName)
{

    int iRet = 0;

    //if already open close and reconnect
    if( m_serialPort->isOpen())
    {
        m_serialPort->close();
    }

    m_serialPort->setPortName(in_strPortName); //default port if user want
    m_serialPort->setBaudRate(QSerialPort::Baud115200);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if(m_serialPort->open(QIODevice::ReadWrite))
    {
        g_pMainwindow->set_serial_link_sts(true);
        m_bIsConnected = true;
    }
    else
    {
        if (m_serialPort->error() == QSerialPort::PermissionError)
        {
            iRet = -1;
        }
        else
        {
            iRet = -2;
        }

        g_pMainwindow->set_serial_link_sts(false);
        m_bIsConnected = false;
    }

    return iRet;
}

bool SerialCore::disconnet_serial()
{
    if (m_serialPort->isOpen())
    {
        m_serialPort->close();
        g_pMainwindow->set_serial_link_sts(false);
        m_bIsConnected = false;
        return true;//successfully closed
    }

    return false;
}

int SerialCore::send_cmd(QByteArray byteArr)
{
    if(m_serialPort->isOpen())
    {
        m_serialPort->write(byteArr);
        m_serialPort->flush();
#ifdef DEBUG_PRINT_ENABLE
        qDebug() << "Sent bytes:" << byteArr.toHex(' ').toUpper();
#endif
        return 0;
    }
    return -1;
}

void SerialCore::slot_read_data()
{
    QByteArray data = m_serialPort->readAll();
    m_rxBuffer.append(data);

    while(m_rxBuffer.contains('\n'))
    {
        int lineEndIndex = m_rxBuffer.indexOf('\n');
        QByteArray line = m_rxBuffer.left(lineEndIndex);
        m_rxBuffer.remove(0, lineEndIndex + 1);
        line = line.trimmed();

        //ais starts with either ! or $
        if(!line.isEmpty() && (line.startsWith('!') || line.startsWith('$')))
        {
            emit signal_parse_rx_buffer(line);
        }
    }

}

void SerialCore::create_objects()
{
    m_serialPort = new QSerialPort(this);
}
