#ifndef BROKENEXPORTER_H
#define BROKENEXPORTER_H

#include "Exporter.h"
#include <QFile>

class BrokenExporter : public Exporter
{
public:
    BrokenExporter();

    // Exporter interface
public:
    bool exportData(const QString &data, const QString &filename);
};

#endif // BROKENEXPORTER_H
