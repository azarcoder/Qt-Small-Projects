#ifndef JSONEXPORTER_H
#define JSONEXPORTER_H

#include "Exporter.h"
#include <QFile>
#include <QTextStream>

class JsonExporter : public Exporter
{
public:
    JsonExporter();

    // Exporter interface
public:
    bool exportData(const QString &data, const QString &filename);
};

#endif // JSONEXPORTER_H
