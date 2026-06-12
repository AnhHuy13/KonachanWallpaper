#include "setting.h"
#include "ui_setting.h"

setting::setting(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::setting)
{
    ui->setupUi(this);
}

setting::~setting()
{
    delete ui;
}

SettingProperties setting::Init() {
    QString appPath = QCoreApplication::applicationDirPath();

    QString settingsFile = QDir(appPath).filePath("settings.ini");

    QSettings settings(settingsFile, QSettings::IniFormat);

    SettingProperties conf;

    if (!settings.contains("cache/storeCache")) {
        // default keys and value
        settings.setValue("cache/storeCache", true);
        settings.setValue("cache/pathCache", QDir(QCoreApplication::applicationDirPath()).filePath("KonachanWallpaper_Cache"));

        settings.setValue("cleanup/autoClean", true);
        settings.setValue("cleanup/autoCleanMin", 60); // the unit is minutes

        settings.setValue("language", "en");
    }

    conf.storeCache = settings.value("cache/storeCache").toBool();
    conf.pathCache = settings.value("cache/pathCache").toString();

    conf.autoClean = settings.value("cache/autoClean").toBool();
    conf.autoCleanMin = settings.value("cache/autoCleanMin").toInt();

    conf.language = settings.value("language").toString();

    return conf;
}

void setting::CreateImagesDir(QString path) {
    if (path == "") {
        QString appDir = QCoreApplication::applicationDirPath();
        QDir().mkpath(QDir(appDir).filePath("KonachanWallpaper_Cache"));
    }
    else {
        QDir().mkpath(path);
    }
}
