#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include "tagchoose.h"
#include "setting.h"
#include <QPlainTextEdit>
#include <QRandomGenerator>
#include <QFileDialog>
#include <QAction>
#include <QToolTip>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void Fetch(int add);

    void DownloadPreviewImage(QString url);

    void DownloadFullImage(QString url);

    void EnableToggleButtons(bool enable);

    void HandleErrorAndFetchAgain(QNetworkReply *reply, QString text, QString informativeText, bool isNeedForShowDetailedText);

    void LoadSettings();

    QByteArray m_fullImageData;

private slots:
    void on_infoBox_clicked();

    void on_findTagsBtn_clicked();

    void handleUpdate();

    void on_generateBtn_clicked();

    void onFetchFinished();

    void on_saveBtn_clicked();

    void on_setWallpaperBtn_clicked();

private:
    Ui::MainWindow *ui;
    TagChoose* dialog;
    QNetworkAccessManager *Manager;
    int m_CurPage;
    int m_AttemptLoading;
    int m_loadCount = 0;
    QString m_fullImageExtension;

    SettingProperties config;
};
#endif // MAINWINDOW_H
