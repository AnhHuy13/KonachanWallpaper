#include "checkimages.h"

int checkimages::CleanUpImage(QString directory, int time)
{
    QDir dir(directory);

    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

    QDateTime timeDelete = QDateTime::currentDateTime().addDays(time * -1);
    QStringList extension = {"JPG","PNG","JPEG","WEBP","BMP"};

    QFileInfoList fileList = dir.entryInfoList();

    int fileRemoved = 0;
    for (int i = 0; i < fileList.size(); ++i) {
        QFileInfo file = fileList.at(i);
        if (extension.contains(file.suffix().toUpper()) && file.lastModified() < timeDelete && IsImageCorrupted(file.absoluteFilePath())) {
            QFile::moveToTrash(file.absoluteFilePath());
            fileRemoved++;
        }
    }
    return fileRemoved;
}

bool checkimages::IsImageCorrupted (QString filePath) {
    QImageReader reader(filePath);
    QSize size = reader.size();

    if (!reader.canRead()) {
        return true;
    }
    if (!size.isValid() || size.width() <= 0 || size.height() <= 0) {
        return true;
    }

    return false;
}
