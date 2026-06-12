#ifndef SETTING_H
#define SETTING_H

#include <QDialog>
#include <QSettings>
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>

namespace Ui {
class setting;
}

struct SettingProperties {
    bool storeCache;
    QString pathCache;

    bool autoClean;
    int autoCleanMin;

    QString language;
};


class setting : public QDialog
{
    Q_OBJECT

public:
    explicit setting(QWidget *parent = nullptr);
    static SettingProperties Init();
    ~setting();

private:
    Ui::setting *ui;

    void CreateImagesDir(QString path);
};

#endif // SETTING_H
