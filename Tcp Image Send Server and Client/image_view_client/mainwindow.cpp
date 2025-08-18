#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QBuffer>
#include <QImageReader>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect( &m_socket, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(tcpError(QAbstractSocket::SocketError)) );
    connect( &m_socket, SIGNAL(readyRead()),this, SLOT(tcpReady()));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    ui->pushButton->setEnabled(false);
    ui->label->setPixmap(QPixmap());
    ui->label->setText(tr("<i> Getting image ... </i>"));
    m_datasize = 0;
    m_socket.abort();
    m_socket.connectToHost(ui->lineEdit->text(), 9876);

}

void MainWindow::tcpReady()
{
    if( m_datasize == 0 )
    {
        QDataStream stream( &m_socket );
        stream.setVersion( QDataStream::Qt_5_12 );
        if( m_socket.bytesAvailable() < sizeof(quint32) )
            return;
        stream >> m_datasize;
    }
    if( m_datasize > m_socket.bytesAvailable() )
        return;
    QByteArray array = m_socket.read( m_datasize );
    QBuffer buffer(&array);
    buffer.open( QIODevice::ReadOnly );
    QImageReader reader(&buffer, "PNG");
    QImage image = reader.read();
    if( !image.isNull() )
    {
        ui->label->clear();
        ui->label->setPixmap( QPixmap::fromImage( image ) );
    }
    else
    {
        ui->label->setText( tr("<i>Invalid image received!</i>") );
    }

    ui->pushButton->setEnabled( true );

}

void MainWindow::tcpError(QAbstractSocket::SocketError error)
{
    if( error == QAbstractSocket::RemoteHostClosedError )
    return;
    QMessageBox::warning( this, tr("Error"),

    tr("TCP error: %1").arg( m_socket.errorString() ) );

    ui->label->setText( tr("<i>No Image</i>") );
    ui->pushButton->setEnabled( true );
}

