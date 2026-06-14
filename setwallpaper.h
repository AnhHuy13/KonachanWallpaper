#ifndef SETWALLPAPER_H
#define SETWALLPAPER_H

#include <QByteArray>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QProcess>
#include <QDateTime>
#include <QCheckBox>
#include "AppData.h"

class SetWallpaper
{
public:
    void DownloadWallpaper(QByteArray img, QString extension, bool isCache, QString directory);

private:
    void setDesktopWallpaper(QString path, bool isCache);

    QString getDesktopEnvironment();

    bool runCommand(const QString& program, const QStringList& args);

    void setDesktopWallpaperFailed(QString path);

signals:
    void setDesktopWallpaperSuccessfully(QString path, bool isCache);
};

#endif // SETWALLPAPER_H
