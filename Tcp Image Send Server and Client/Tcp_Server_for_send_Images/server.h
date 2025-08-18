#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>

class Server : public QTcpServer
{
public:
    Server();

    // QTcpServer interface
protected:
    void incomingConnection(qintptr handle);
};

#endif // SERVER_H
