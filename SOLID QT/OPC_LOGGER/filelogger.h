#ifndef FILELOGGER_H
#define FILELOGGER_H

#include <QObject>
#include "Logger.h"

class FileLogger : public QObject, public Logger
{
    Q_OBJECT
public:
    explicit FileLogger(QObject *parent = nullptr);

signals:


    // Logger interface
public:
    void log(QString &msg);
};

#endif // FILELOGGER_H
