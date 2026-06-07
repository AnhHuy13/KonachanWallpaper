#ifndef CHECKIMAGES_H
#define CHECKIMAGES_H

#include <QDir>
#include <QDateTime>
#include <QStringList>
#include <QFileInfo>
#include <QFile>
#include <QFileInfoList>
#include <QImageReader>

class checkimages
{
public:
    void CleanUpImage(QString directory, int time);

    bool IsImageCorrupted(QString filePath);
};

#endif // CHECKIMAGES_H
