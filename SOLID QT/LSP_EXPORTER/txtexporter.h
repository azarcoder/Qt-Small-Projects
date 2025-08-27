#ifndef TXTEXPORTER_H
#define TXTEXPORTER_H

#include "Exporter.h"
#include <QFile>
#include <QTextStream>

class TxtExporter : public Exporter
{
public:
    TxtExporter();

    // Exporter interface
public:
    bool exportData(const QString &data, const QString &filename) override;
};

#endif // TXTEXPORTER_H
