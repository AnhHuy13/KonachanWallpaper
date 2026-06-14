#include "setwallpaper.h"
#include "AppData.h"
#include "qcheckbox.h"
#ifdef Q_OS_WIN
#include <windows.h>
#include <winuser.h>
#endif

void SetWallpaper::DownloadWallpaper(QByteArray img, QString extension, bool isCache, QString directory) {
    if (img.isEmpty()) return;


    QDir dir(directory);

    QString filePath = directory + "/" + QDateTime::currentDateTime().toString("dd-MM-yyyy_hh-mm-ss") +"." + extension;
    QFile file(filePath);

    if (file.open(QIODevice::WriteOnly)) {
        file.write(img);
        file.close();
        setDesktopWallpaper(filePath, isCache);
    }
}

QString getDesktopEnvironment() {
    // for linux desktop environment
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString de = env.value("XDG_CURRENT_DESKTOP");

    if (de.isEmpty()) {
        return "UNKNOWN";
    }
    return de;
}

void SetWallpaper::setDesktopWallpaperFailed(QString path) {
    QMessageBox msgBox;
    msgBox.setText(msgBox.tr("Sorry, your OS or Desktop Environment isn't supported yet..."));
    msgBox.setInformativeText(msgBox.tr("Here is your image path file: ")+path+msgBox.tr("  Please manually set the wallpaper."));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}

void SetWallpaper::setDesktopWallpaperSuccessfully(QString path, bool isCache) {
    if (!isCache) {
        if (QFile::remove(path)) {
            qDebug() << "file " + path + " removed";
        } else {
            if (!AppData::instance().ignoreCacheErrors) {
                QMessageBox msgBox;

                QCheckBox *ignoreCheck = new QCheckBox(tr("Ignore future cache errors"), &msgBox);
                msgBox.setCheckBox(ignoreCheck);

                msgBox.setIcon(QMessageBox::Critical);
                msgBox.setText("Can't remove cache file.");
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.setDefaultButton(QMessageBox::Ok);
                msgBox.exec();

                if (ignoreCheck->isChecked()) {
                    AppData::instance().ignoreCacheErrors = true;
                }
            }
        }
    }
}

bool SetWallpaper::runCommand(const QString& program, const QStringList& args) {
    QProcess process;
    process.start(program, args);
    return process.waitForFinished() && (process.exitCode() == 0);
}

void SetWallpaper::setDesktopWallpaper(QString path, bool isCache) {

#ifdef Q_OS_DARWIN
    QString filePath = QDir::toNativeSeparators(path);
    QString script = QString("tell application \"System Events\" to set picture of every desktop to \"%1\"").arg(path);
    runCommand("osascript", {"-e", script});

#elif defined(Q_OS_WIN)
    QString filePath = QDir::toNativeSeparators(path);
    int ret = SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (void*)filePath.toStdWString().c_str(), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);

#elif defined(Q_OS_LINUX)
    // DE supported in this code:
    // KDE, GNOME, XFCE, Cinnamon, MATE, Budgie, Pantheon, LXQt, Deepin, I3WM

    QString de = getDesktopEnvironment().toUpper();
    QString filePath = QDir::toNativeSeparators(path);
    if (de.contains("KDE")) {
        QProcess process;
        QString script = QString(R"(
                                 var allDesktops = desktops();
                                 for (i=0; i<allDesktops.length; i++) {
                                 var d = allDesktops[i];
                                 d.wallpaperPlugin = "org.kde.image";
                                 d.currentConfigGroup = ["Wallpaper", "org.kde.image", "General"];
                                 d.writeConfig("Image", "file://%1");
                                 }
                                 )").arg(filePath);

        process.start("qdbus", {"org.kde.plasmashell", "/PlasmaShell", "org.kde.PlasmaShell.evaluateScript", script});
        process.waitForFinished();
        int exitCode = process.exitCode();
        if (exitCode != 0) {

        }
    }
    else if (de.contains("GNOME") || de.contains("BUDGIE") || de.contains("PANTHEON")) {
        SetWallpaper::runCommand("gsettings", {"set", "org.gnome.desktop.background", "picture-uri", "file://" + filePath});
    }
    else if (de.contains("CINNAMON")) {
        SetWallpaper::runCommand("gsettings", {"set", "org.cinnamon.desktop.background", "picture-uri", "file://" + filePath});
    }
    else if (de.contains("MATE")) {
        SetWallpaper::runCommand("gsettings", {"set", "org.mate.desktop.background", "picture-uri", "file://" + filePath});
    }
    else if (de.contains("DEEPIN")) {
        SetWallpaper::runCommand("gsettings", {"set", "com.deepin.wrap.gnome.desktop.background", "picture-uri", "file://" + filePath});
    }
    else if (de.contains("LXQT")) {
        SetWallpaper::runCommand("pcmanfm-qt", {"--set-wallpaper=" + filePath});
    }
    else if (de.contains("XFCE")) {
        SetWallpaper::runCommand("xfconf-query", {"-c", "xfce4-desktop", "-p", "/backdrop/screen0/monitor0/workspace0/last-image", "-s", filePath});
        SetWallpaper::runCommand("xfdesktop", {"--reload"});
    }
    else if (de.contains("I3WM")) {
        SetWallpaper::runCommand("feh", {"--bg-scale", filePath});
    }
    else {
        qDebug() << "Unsupported or unknown DE:" << de;
        emit SetWallpaper::setDesktopWallpaperFailed(filePath);
    }

#elif defined(Q_OS_BSD4)

    QString filePath = QDir::toNativeSeparators(path);
    if (SetWallpaper::runCommand("feh", {"--bg-scale", filePath}) != 0) {
        emit SetWallpaper::setDesktopWallpaperFailed(filePath);
    }

#else

#error "OS or DE isn't supported yet..."
    QString filePath = QDir::toNativeSeparators(path);
    emit SetWallpaper::setDesktopWallpaperFailed(filePath);

#endif
    emit SetWallpaper::setDesktopWallpaperSuccessfully(filePath, isCache);
}
