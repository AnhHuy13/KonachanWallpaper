#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tagchoose.h"
#include <QToolTip>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFocus();
    this->setFixedSize(this->size().width(), this->size().height());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_infoBox_clicked()
{
    int selectedIndex = ui->safetyBox->currentIndex();
    if (selectedIndex == 0) {
        QString tip = "<b>Wholesome/safe content</b>: includes:"
                      "<ul> <li>landscape photos</li><li>anime</li><li>normal fanart/official art</li> <li>etc.</li></ul>";

        ui->infoBox->setToolTip(tip);

        QToolTip::showText(ui->infoBox->mapToGlobal(ui->infoBox->rect().center()), tip, ui->infoBox, QRect(), 2000);
    }
    else if (selectedIndex == 1) {
        QString tip = "<b>Suggestive content:</b> includes:"
                      "<ul> <li>lingerie</li><li>swimwear</li><li>in provocative poses<li> <li>etc.</li></ul>";

        ui->infoBox->setToolTip(tip);

        QToolTip::showText(ui->infoBox->mapToGlobal(ui->infoBox->rect().center()), tip, ui->infoBox, QRect(), 2000);
    }
    else if (selectedIndex == 2) {
        QString tip = "<b>Adult content: </b> Includes nudity or highly sensitive material.";

        ui->infoBox->setToolTip(tip);

        QToolTip::showText(ui->infoBox->mapToGlobal(ui->infoBox->rect().center()), tip, ui->infoBox, QRect(), 2000);
    }
}

void MainWindow::on_findTagsBtn_clicked()
{
    TagChoose *dialog = new TagChoose(this);
    dialog->show();
    dialog->GetApi("");
}
