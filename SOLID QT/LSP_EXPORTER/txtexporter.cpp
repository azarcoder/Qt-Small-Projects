#include "txtexporter.h"

TxtExporter::TxtExporter()
{

}

bool TxtExporter::exportData(const QString &data, const QString &filename)
{
    QFile file(filename + ".txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out << data;
    return true;
}
