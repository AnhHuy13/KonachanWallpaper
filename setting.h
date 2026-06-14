#ifndef SETTING_H
#define SETTING_H

#include <QDialog>
#include <QSettings>
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <QFileDialog>

namespace Ui {
class setting;
}

struct SettingProperties {
    bool isStoreCache;
    QString pathCache;

    bool isAutoClean;
    int autoCleanSec;

    QString language;
};

inline const QString KEY_IS_STORE_CACHE = "cache/isStoreCache";
inline const QString KEY_PATH_CACHE = "cache/pathCache";
inline const QString KEY_IS_AUTO_CLEAN = "cleanup/isAutoClean";
inline const QString KEY_AUTO_CLEAN_SEC = "cleanup/autoCleanSec";
inline const QString KEY_LANGUAGE = "general/language";

class setting : public QDialog
{
    Q_OBJECT

public:
    explicit setting(QWidget *parent = nullptr);
    static void Init();
    void TakeData();
    static SettingProperties ReadData();
    ~setting();

signals:
    void SettingsChanged();

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_changeWallpaperLocationBtn_clicked();

private:
    Ui::setting *ui;
    QString directoryChoose;

    void CreateImagesDir(QString path);

};

#endif // SETTING_H
