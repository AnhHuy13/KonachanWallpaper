#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include "tagchoose.h"
#include <QPlainTextEdit>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void Fetch();


private slots:
    void on_infoBox_clicked();

    void on_findTagsBtn_clicked();

    void handleUpdate();

    void on_generateBtn_clicked();


private:
    Ui::MainWindow *ui;
    TagChoose* dialog;
    QNetworkAccessManager *Manager;
};
#endif // MAINWINDOW_H
