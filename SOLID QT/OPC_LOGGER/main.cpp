#include <QCoreApplication>

#include "filelogger.h"
#include "consolelogger.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    FileLogger fl;
    ConsoleLogger cl;

    QString s = "Sample Message";
    fl.log(s);
    cl.log(s);

    return a.exec();
}
