#ifndef SETWALLPAPER_H
#define SETWALLPAPER_H

#include <QByteArray>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QProcess>
#include <QDateTime>

class SetWallpaper
{
public:
    void DownloadWallpaper(QByteArray img, QString extension);

private:
    void setDesktopWallpaper(QString path);

    QString getDesktopEnvironment();

    bool runCommand(const QString& program, const QStringList& args);

    void setDesktopWallpaperFailed(QString path);

};

#endif // SETWALLPAPER_H
