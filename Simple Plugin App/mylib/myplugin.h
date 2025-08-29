#ifndef MYPLUGIN_H
#define MYPLUGIN_H

#include "myinterface.h"
#include <QObject>

class MyPlugin : public QObject, public MyInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID MyInterface_iid)  // Register metadata
    Q_INTERFACES(MyInterface)               // Tell Qt we implement this interface

public:
    QString sayHello() const override {
        return "Hello from Plugin!";
    }
};

#endif // MYPLUGIN_H
