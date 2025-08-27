#include "brokenexporter.h"

BrokenExporter::BrokenExporter()
{

}

bool BrokenExporter::exportData(const QString &data, const QString &filename)
{
    // BAD: violates base contract
    if (data.contains(" ")) {
        return false; // refuses valid input
    }

    QFile file(filename + ".txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    file.write(data.toUtf8());

    // BAD: deletes file after writing (violates expectation)
    file.remove();
    return true;
}
