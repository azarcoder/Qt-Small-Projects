#include "consolelogger.h"
#include <QDebug>

ConsoleLogger::ConsoleLogger(QObject *parent) : QObject(parent)
{

}

void ConsoleLogger::log(QString &msg)
{
    qDebug() << "loggin console message : " << msg;
}
