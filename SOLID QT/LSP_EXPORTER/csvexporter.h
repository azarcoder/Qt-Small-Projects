#ifndef CSVEXPORTER_H
#define CSVEXPORTER_H

#include "Exporter.h"
#include <QFile>
#include <QTextStream>

class CsvExporter : public Exporter
{
public:
    CsvExporter();

    // Exporter interface
public:
    bool exportData(const QString &data, const QString &filename);
};

#endif // CSVEXPORTER_H
