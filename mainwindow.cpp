#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tagchoose.h"
#include "AppData.h"
#include <QToolTip>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFocus();
    this->setFixedSize(this->size().width(), this->size().height());

    AppData::instance().lastTagsSelected.clear();
    Manager = new QNetworkAccessManager(this);
    qDebug() << AppData::instance().TagsSelected;


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

    connect(dialog, &TagChoose::tagsUpdated, this, &MainWindow::handleUpdate);

    dialog->show();
    dialog->GetApi("");
}

void MainWindow::handleUpdate() {
    QString tags = AppData::instance().TagsSelected.join(", ");
    ui->txtChosenTags->QPlainTextEdit::setPlainText(tags);
    AppData::instance().lastTagsSelected = AppData::instance().TagsSelected;
}

void MainWindow::on_generateBtn_clicked()
{
    if (AppData::instance().TagsSelected.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setText("You should choose atleast 1 tag.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        int ret = msgBox.exec();
    }
    else {
        Fetch();
    }
}

void MainWindow::Fetch() {
    QString APIUrlStr;
    APIUrlStr.append("ttps://konachan.net/post.json?limit=1&random=1&tags=");
    APIUrlStr.append(QString(AppData::instance().TagsSelected.join(+)));
    APIUrlStr.append("+rating:");
    APIUrlStr.append(ui->safetyBox->);


    QUrl APIUrl;
    QString urlText = QString("https://konachan.net/post.json?limit=1&random=1&tags=hatsune_miku+maid+rating:explicit");
    APIUrl = QUrl(urlText);

    QNetworkRequest request(APIUrl);
    QNetworkReply *reply = MainWindow::Manager->get(request);

    // connect(reply, &QNetworkReply::finished, this, &TagChoose::onSearchFinished);
}


