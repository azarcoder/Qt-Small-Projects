#include "serverthread.h"
#include <QTime>
#include <QDir>
#include <QTcpSocket>
#include <QBuffer>
#include <QImageWriter>

ServerThread::ServerThread(int descriptor, QObject *parent) : QThread(parent)
{
    m_descriptor = descriptor; //m_descriptor holds the OS‐level socket identifier.
}

//Thread’s Main Loop
//When you call thread->start(), Qt invokes run() on a new thread.
void ServerThread::run()
{
    QTcpSocket socket;
    if( !socket.setSocketDescriptor( m_descriptor ) )
    {
        qDebug( "Socket error!" );
        return;
    }
    QBuffer buffer; //as an in-memory PNG container.
    QImageWriter writer(&buffer, "PNG"); //writes the QImage into buffer.
    writer.write( randomImage() );

    //frame data
    QByteArray data;
    QDataStream stream( &data, QIODevice::WriteOnly );
    stream.setVersion( QDataStream::Qt_5_12);
    stream << (quint32)buffer.data().size();
    data.append( buffer.data() );

    //pushes the entire packet to the client.
    socket.write( data );

//    qDebug() << data;

    socket.disconnectFromHost();
    socket.waitForDisconnected();
}

QImage ServerThread::randomImage()
{
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    QDir dir("images"); //to filter and list all files in images/
    dir.setFilter( QDir::Files );
    QFileInfoList entries = dir.entryInfoList();
    if( entries.size() == 0 )
    {
        qDebug( "No images to show!" );
        return QImage();
    }
    return QImage( entries.at( qrand() % entries.size() ).absoluteFilePath() );
}
