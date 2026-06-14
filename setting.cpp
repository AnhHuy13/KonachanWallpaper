#include "setting.h"
#include "ui_setting.h"

setting::setting(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::setting)
{
    ui->setupUi(this);

    QString appPath = QCoreApplication::applicationDirPath();

    QString settingsFile = QDir(appPath).filePath("settings.ini");

    QSettings settings(settingsFile, QSettings::IniFormat);

    ui->enableCache->setChecked(settings.value(KEY_IS_STORE_CACHE).toBool());
    ui->wallpaperLocationLbl->setText("Location:" + (settings.value(KEY_PATH_CACHE).toString()));

    setting::directoryChoose = settings.value(KEY_PATH_CACHE).toString();

    ui->enableCleanUpAuto->setChecked(settings.value(KEY_IS_AUTO_CLEAN).toBool());

    int timeToSecondInInit = (settings.value(KEY_AUTO_CLEAN_SEC).toInt());

    // các mốc thời gian trong autoCleanMin (đơn vị Giây):
    // 30 phút(1800), 1 giờ(3600), 12 giờ(43200), 1 ngày(86400)
    // 1 tuần(604800), 2 tuần(1209600), 1 tháng(2592000), 6 tháng(15778800)

    const QList<int> timeMap = {1800, 3600, 43200, 86400, 604800, 1209600, 2592000, 15778800};
    int timeChosenInInit = timeMap.indexOf(timeToSecondInInit);
    if (timeChosenInInit == -1) {
        timeChosenInInit = 0;
    }
    ui->timeToClean->setCurrentIndex(timeChosenInInit);

    QString languageInInit = settings.value(KEY_LANGUAGE, "en").toString();
    const QMap<QString, int> langMapInit = {{"en", 0}, {"vi", 1}, {"ja", 2}};
    ui->language->setCurrentIndex(langMapInit.value(languageInInit, 0));

    ui->changeWallpaperLocationBtn->setEnabled(ui->enableCache->isChecked());
    ui->wallpaperLocationLbl->setEnabled(ui->enableCache->isChecked());
    ui->label_5->setEnabled(ui->enableCache->isChecked());
    ui->timeToClean->setEnabled(ui->enableCleanUpAuto->isChecked());
    ui->label->setEnabled(ui->enableCleanUpAuto->isChecked());

    connect(ui->enableCache, &QCheckBox::toggled, ui->changeWallpaperLocationBtn, &QWidget::setEnabled);
    connect(ui->enableCache, &QCheckBox::toggled, ui->wallpaperLocationLbl, &QWidget::setEnabled);
    connect(ui->enableCache, &QCheckBox::toggled, ui->label_5, &QWidget::setEnabled);
    connect(ui->enableCleanUpAuto, &QCheckBox::toggled, ui->timeToClean, &QWidget::setEnabled);
    connect(ui->enableCleanUpAuto, &QCheckBox::toggled, ui->label, &QWidget::setEnabled);
}

setting::~setting()
{
    delete ui;
}

void setting::Init() {
    QString appPath = QCoreApplication::applicationDirPath();

    QString settingsFile = QDir(appPath).filePath("settings.ini");

    QSettings settings(settingsFile, QSettings::IniFormat);

    if (!settings.contains(KEY_LANGUAGE)) {
        settings.setValue(KEY_IS_STORE_CACHE, true);
        settings.setValue(KEY_PATH_CACHE, QDir(QCoreApplication::applicationDirPath()).filePath("KonachanWallpaper_Cache"));

        settings.setValue(KEY_IS_AUTO_CLEAN, true);
        settings.setValue(KEY_AUTO_CLEAN_SEC, 3600);

        settings.setValue(KEY_LANGUAGE, "en");
    }
}

SettingProperties setting::ReadData() {
    QString appPath = QCoreApplication::applicationDirPath();

    QString settingsFile = QDir(appPath).filePath("settings.ini");

    QSettings settings(settingsFile, QSettings::IniFormat);

    SettingProperties conf;

    conf.isStoreCache = settings.value(KEY_IS_STORE_CACHE).toBool();
    conf.pathCache = settings.value(KEY_PATH_CACHE).toString();
    conf.isAutoClean = settings.value(KEY_IS_AUTO_CLEAN).toBool();
    conf.autoCleanSec = settings.value(KEY_AUTO_CLEAN_SEC).toInt();
    conf.language = settings.value(KEY_LANGUAGE).toString();

    return conf;
}

void setting::TakeData() {
    QString appPath = QCoreApplication::applicationDirPath();

    QString settingsFile = QDir(appPath).filePath("settings.ini");

    QSettings settings(settingsFile, QSettings::IniFormat);

    settings.setValue(KEY_IS_STORE_CACHE, ui->enableCache->isChecked());

    settings.setValue(KEY_PATH_CACHE, setting::directoryChoose);

    settings.setValue(KEY_IS_AUTO_CLEAN, ui->enableCleanUpAuto->isChecked());

    int timeChoose = ui->timeToClean->currentIndex();

    // các mốc thời gian trong autoCleanMin (đơn vị Giây):
    // 30 phút(1800), 1 giờ(3600), 12 giờ(43200), 1 ngày(86400)
    // 1 tuần(604800), 2 tuần(1209600), 1 tháng(2592000), 6 tháng(15778800)

    const QList<int> timeMap = {1800, 3600, 43200, 86400, 604800, 1209600, 2592000, 15778800};
    int timeToSecs = (timeChoose >= 0 && timeChoose < timeMap.size()) ? timeMap.at(timeChoose) : 1800;

    settings.setValue(KEY_AUTO_CLEAN_SEC, timeToSecs);

    int langChoose = ui->language->currentIndex();
    const QMap<int, QString> langMapTake = {{0, "en"}, {1, "vi"}, {2, "ja"}};
    QString language = langMapTake.value(langChoose, "en");

    settings.setValue(KEY_LANGUAGE, language);
}

void setting::CreateImagesDir(QString path) {
}

void setting::on_buttonBox_accepted()
{
    setting::TakeData();
    this->done(QDialog::Accepted);
    emit SettingsChanged();
}

void setting::on_buttonBox_rejected()
{
    this->close();
}

void setting::on_changeWallpaperLocationBtn_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Choose a folder", QDir::homePath());
    if (!dir.isEmpty()) {
        setting::directoryChoose = dir;
        ui->wallpaperLocationLbl->setText("Location:" + setting::directoryChoose);
    }
}