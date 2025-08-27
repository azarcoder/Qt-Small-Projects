#ifndef CONSOLELOGGER_H
#define CONSOLELOGGER_H

#include "Logger.h"
#include <QObject>

class ConsoleLogger : public QObject, public Logger
{
    Q_OBJECT
public:
    explicit ConsoleLogger(QObject *parent = nullptr);

signals:


// Logger interface
public:
    void log(QString &msg) override;
};

#endif // CONSOLELOGGER_H
