#ifndef MYINTERFACE_H
#define MYINTERFACE_H

#include <QtPlugin>

// Define a pure virtual interface
class MyInterface {
public:
    virtual ~MyInterface() {}
    virtual QString sayHello() const = 0;
};

// Unique ID for this interface
#define MyInterface_iid "org.example.MyInterface"

// Register interface with Qt
Q_DECLARE_INTERFACE(MyInterface, MyInterface_iid)

#endif // MYINTERFACE_H
