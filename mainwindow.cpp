#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tagchoose.h"
#include "setwallpaper.h"
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
    this->m_fullImageExtension = "";

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::EnableToggleButtons(bool enable){
    ui->generateBtn->setEnabled(enable);
    ui->setWallpaperBtn->setEnabled(enable);
    ui->saveBtn->setEnabled(enable);
}

void MainWindow::HandleErrorAndFetchAgain(QNetworkReply *reply) {
    this->EnableToggleButtons(true);
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText("An error has occured.");
    msgBox.setInformativeText("Please try again few more times.");
    msgBox.setDetailedText(reply->errorString());
    QPushButton* okBtn = msgBox.addButton(tr("Ok"),QMessageBox::ActionRole);
    msgBox.exec();

    if (msgBox.clickedButton() == okBtn)  {
        this->EnableToggleButtons(true);
        msgBox.accept();
    }

    qDebug() << "Error :" << reply->errorString();
    this->EnableToggleButtons(true);

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
    this->EnableToggleButtons(false);
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
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/78.0.3904.108 Safari/537.36 OPR/65.0.3467.78 (Edition Campaign 70)");
    request.setRawHeader("Referer", "https://konachan.net/");
    QNetworkReply *reply = this->Manager->get(request);

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
                if (obj.value("jpeg_url").toString().isEmpty() == false) {
                    QString fullUrl = obj.value("jpeg_url").toString(); //full img
                    qDebug() << "jpeg_url: " << fullUrl;
                    this->DownloadFullImage(fullUrl);
                }
                else {
                    this->m_CurPage = 0;
                    QMessageBox msgBox;
                    this->EnableToggleButtons(true);
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
            this->HandleErrorAndFetchAgain(reply);
        }
        reply->deleteLater();
    }
}
void MainWindow::DownloadPreviewImage(QString url) {
    /*
    QNetworkRequest request((QUrl(url)));
    request.setTransferTimeout(5000);
    QNetworkReply *reply = this->Manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "Đang tải Preview image";
            QPixmap pm;
            if (pm.loadFromData(reply->readAll())){
                ui->previewImg->setPixmap(pm.scaled(ui->previewImg->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                m_loadCount++;
                if (m_loadCount >= 2) {
                    this->EnableToggleButtons(true);
                }
            }
            else {
                qDebug() << "image is corrupted";
            }
        }
        else {
            qDebug() << "Lỗi ở Preview image!";
            this->EnableToggleButtons(true);
            this->HandleErrorAndFetchAgain(reply);
        }
        reply->deleteLater();
    });
    */
}

void MainWindow::DownloadFullImage(QString url) {
    QFileInfo fileInfo(url);
    this->m_fullImageExtension = fileInfo.suffix();
    QNetworkRequest request((QUrl(url)));
    request.setTransferTimeout(5000);
    QNetworkReply *reply = this->Manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "Đang tải full image!";
            QByteArray data = reply->readAll();
            this->m_fullImageData = data;
            this->EnableToggleButtons(true);
            QPixmap pm;
            if (pm.loadFromData(data)){
                ui->previewImg->setPixmap(pm.scaled(ui->previewImg->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            }

       }

        else {
            qDebug() << "Lỗi ở Full image!";
            this->EnableToggleButtons(true);
            this->HandleErrorAndFetchAgain(reply);
        }
        reply->deleteLater();
    });
}


void MainWindow::on_saveBtn_clicked()
{
    if (m_fullImageData.isEmpty()) {return;};

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save image"), "", tr("Images (*.jpg *.png *.jpeg)"));

    if (!fileName.isEmpty()) {
        this->statusBar()->showMessage("Saving image...");
        QImage img = QImage::fromData(m_fullImageData);
        if (!img.isNull()) {
            img.save(fileName);
            this->statusBar()->showMessage("Image saved successfully at "+ fileName, 3000);
        }
        else {
            QMessageBox::critical(this, "Error", "Could not save file");
        }
    }
}

void MainWindow::on_setWallpaperBtn_clicked()
{
    SetWallpaper idk;
    idk.DownloadWallpaper(this->m_fullImageData,this->m_fullImageExtension);
}
