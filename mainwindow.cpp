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

    AppData::instance().lastTagsSelected.clear();
    Manager = new QNetworkAccessManager(this);
    qDebug() << AppData::instance().TagsSelected;
    ui->safetyBox->setItemData(0,"safe");
    ui->safetyBox->setItemData(1,"questionable");
    ui->safetyBox->setItemData(2,"explicit");

    this->setFocus();
    this->setFixedSize(this->size().width(), this->size().height());
    this->m_CurPage = 0;
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
        msgBox.exec();
    }
    else {
        Fetch();
    }
}

void MainWindow::Fetch() {
    ui->generateBtn->setEnabled(false);
    m_CurPage++;
    QString APIUrlStr;
    APIUrlStr.append("https://konachan.net/post.json?limit=50&random=1&page="); //limit 50
    APIUrlStr.append(QString::number(m_CurPage));
    APIUrlStr.append("&tags=");
    APIUrlStr.append(AppData::instance().TagsSelected.join("+"));
    APIUrlStr.append("+rating:");
    APIUrlStr.append(ui->safetyBox->currentData().toString());
    qDebug() << APIUrlStr;

    QUrl APIUrl;
    APIUrl = QUrl(APIUrlStr);

    QNetworkRequest request(APIUrl);
    QNetworkReply *reply = MainWindow::Manager->get(request);

    connect(reply, &QNetworkReply::finished, this, &MainWindow::onFetchFinished);
}

void MainWindow::onFetchFinished() {
    qDebug() << "is loading";
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    if (reply->error() == QNetworkReply::NoError){
        QByteArray responseData = reply->readAll();
        QJsonDocument JDoc = QJsonDocument::fromJson(responseData);

        if (JDoc.isArray()) {
            QJsonArray JArray = JDoc.array();
            if (!JArray.isEmpty()) {
                int randomIndex = QRandomGenerator::global()->bounded(JArray.size());
                QJsonObject obj = JArray.at(randomIndex).toObject();
                QString sampleUrl = obj.value("sample_url").toString();
                qDebug() << "sample_url: " << sampleUrl;
                MainWindow::DownloadImage(sampleUrl,true);
                QJsonObject obj1 = JArray.at(randomIndex).toObject();
                QString fullUrl = obj.value("jpeg_url").toString(); //highest res
                qDebug() << "jpeg_url: " << fullUrl;
                MainWindow::DownloadImage(fullUrl,false);
            }
            else {
                this->m_CurPage = 0;
                QMessageBox msgBox;
                ui->generateBtn->setEnabled(true);
                msgBox.setIcon(QMessageBox::Critical);
                msgBox.setText("An error has occured.");
                msgBox.setInformativeText("<ul>"
                                          " <li>Your selected tags might be too specific or incompatible.</li>"
                                          " <li>Your rating (Safe/Questionable/Explicit) might filtering all images.</li>"
                                          " <li>Server might be busy.</li>"
                                          " <li>You might need to loosen your tags or ratings.</li>"
                                          "</ul>");
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.setDefaultButton(QMessageBox::Ok);
                msgBox.exec();
            }
        }
    }
    else {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText("An error has occured.");
        msgBox.setInformativeText("Please try again few more times.");
        msgBox.setDetailedText(reply->errorString());
        QPushButton* okBtn = msgBox.addButton(tr("Try Again"),QMessageBox::ActionRole);
        QPushButton* cancelBtn = msgBox.addButton(tr("Cancel"),QMessageBox::ActionRole);
        msgBox.exec();

        if (msgBox.clickedButton() == okBtn) {
            MainWindow::Fetch();
        }
        else if (msgBox.clickedButton() == cancelBtn)  {
            this->close();
        }

        qDebug() << "Error :" << reply->errorString();
        ui->generateBtn->setEnabled(true);
    }
    reply->deleteLater();
}

void MainWindow::DownloadImage(QString url, bool isSetPixmap) {
    QNetworkRequest request((QUrl(url)));
    QNetworkReply *reply = this->Manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            if (isSetPixmap) {
                QPixmap pm;
                if (pm.loadFromData(data)){
                    ui->previewImg->setPixmap(pm.scaled(ui->previewImg->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    ui->generateBtn->setEnabled(true);
                }else {
                    qDebug() << "image is corrupted";
                }

            }
            else {
                this->m_fullImageData = data;
            }
        }
        else {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setText("An error has occured.");
            msgBox.setInformativeText("Please try again few more times.");
            msgBox.setDetailedText(reply->errorString());
            QPushButton* okBtn = msgBox.addButton(tr("Try Again"),QMessageBox::ActionRole);
            QPushButton* cancelBtn = msgBox.addButton(tr("Cancel"),QMessageBox::ActionRole);
            msgBox.exec();

            if (msgBox.clickedButton() == okBtn) {
                MainWindow::Fetch();
            }
            else if (msgBox.clickedButton() == cancelBtn)  {
                this->close();
            }

            qDebug() << "Error :" << reply->errorString();
            ui->generateBtn->setEnabled(true);
        }
        reply->deleteLater();
    });
}

void MainWindow::on_saveBtn_clicked()
{
    if (m_fullImageData.isEmpty()) {return;};

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save image"), "", tr("Images (*.jpg)"));

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(m_fullImageData);
            file.close();
        }
    }
}
