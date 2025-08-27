#ifndef LOGGER_H
#define LOGGER_H

#include <QString>

class Logger
{
    virtual void log(QString &msg) = 0;
};

#endif // LOGGER_H
