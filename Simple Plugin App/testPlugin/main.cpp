#include <QCoreApplication>
#include <QDebug>
#include <QPluginLoader>
#include "myinterface.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    //once compile and put it in debug folder
    QPluginLoader loader("myplugin");  // Path to DLL/SO
    QObject *plugin = loader.instance();

    if (plugin) {
        MyInterface *iface = qobject_cast<MyInterface*>(plugin);
        if (iface) {
            qDebug() << "AnotherApp says:" << iface->sayHello();
        }
    } else {
        qDebug() << "Error loading plugin:" << loader.errorString();
    }
    return a.exec();
}
