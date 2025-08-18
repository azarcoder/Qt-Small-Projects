#include "server.h"
#include "serverthread.h"
Server::Server()
{

}

void Server::incomingConnection(qintptr handle)
{
    /* By offloading each client to its own QThread,
    you ensure the server can immediately go back to listening for more connections.

    When a client connects, the OS hands you this descriptor so you can build a QTcpSocket around it.
*/

    ServerThread *thread = new ServerThread(handle, this);
    connect(thread, &ServerThread::finished, thread, &ServerThread::deleteLater);
    thread->start();
}
