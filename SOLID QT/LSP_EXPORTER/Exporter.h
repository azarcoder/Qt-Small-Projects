#ifndef EXPORTER_H
#define EXPORTER_H

#include <QString>

class Exporter {
public:
    virtual ~Exporter() {}
    virtual bool exportData(const QString& data, const QString& filename) = 0;
};

#endif // EXPORTER_H
