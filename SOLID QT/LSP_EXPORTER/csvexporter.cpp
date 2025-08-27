#include "csvexporter.h"

CsvExporter::CsvExporter()
{

}

bool CsvExporter::exportData(const QString &data, const QString &filename)
{
    QFile file(filename + ".csv");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    QString csv = data;
    csv.replace(" ", ","); // simple example: replace spaces with commas
    out << csv;
    return true;
}
