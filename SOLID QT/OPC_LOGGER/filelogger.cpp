#include "filelogger.h"

#include <QDebug>

FileLogger::FileLogger(QObject *parent) : QObject(parent)
{

}

void FileLogger::log(QString &msg)
{
 qDebug() << "logginf file : " << msg;
}
