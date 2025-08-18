#include <QCoreApplication>
#include "server.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Server server;
//    listen for incoming connections on port 9876.
    if(!server.listen(QHostAddress::Any, 9876))
    {
        qCritical("Cannot listen to port, 9876");
        return -1;
    }
    else
    {
        qDebug() << "server listening in port 9876...";
    }

    return a.exec();
}
