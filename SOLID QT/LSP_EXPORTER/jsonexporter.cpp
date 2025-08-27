#include "jsonexporter.h"

JsonExporter::JsonExporter()
{

}

bool JsonExporter::exportData(const QString &data, const QString &filename)
{
    QFile file(filename + ".json");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out << "{ \"data\": \"" << data << "\" }";
    return true;

}
