#include "mainwindow.h"
#include "qstatusbar.h"
#include "ui_mainwindow.h"
#include "tagchoose.h"
#include "setwallpaper.h"
#include "AppData.h"
#include "setting.h"
#include "checkimages.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("KonachanWallpaper");

    if (ui->centralwidget->layout()) {
        ui->centralwidget->layout()->setSizeConstraint(QLayout::SetFixedSize);
    }

    this->setMinimumSize(this->sizeHint());
    this->setMaximumSize(this->sizeHint());

    AppData::instance().lastTagsSelected.clear();
    Manager = new QNetworkAccessManager(this);
    qDebug() << AppData::instance().TagsSelected;
    ui->safetyBox->setItemData(0,"safe");
    ui->safetyBox->setItemData(1,"questionable");
    ui->safetyBox->setItemData(2,"explicit");

    this->setFocus();
    this->m_CurPage = 0;
    this->m_fullImageExtension = "";

    QString nameSetting;

#ifdef Q_OS_MAC
    nameSetting = tr("Preferences...");
#else
    nameSetting = tr("Settings...");
#endif

    QAction *settingAction = new QAction(tr(qUtf8Printable(nameSetting)), this);
    settingAction->setShortcut(QKeySequence::Preferences);
    settingAction->setMenuRole(QAction::PreferencesRole);

    connect(settingAction, &QAction::triggered, this, [=]() {
        setting settingWindow(this);
        settingWindow.exec();
    });

    ui->menuKonachanWallpaper->addAction(settingAction);

    this->LoadSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::LoadSettings() {
    config = setting::ReadData();
    if (config.isAutoClean == true) {
        checkimages::CleanUpImage(config.pathCache, config.autoCleanSec);
    }
}

void MainWindow::EnableToggleButtons(bool enable){
    ui->generateBtn->setEnabled(enable);
    ui->setWallpaperBtn->setEnabled(enable);
    ui->saveBtn->setEnabled(enable);
}

void MainWindow::HandleErrorAndFetchAgain(QNetworkReply *reply, QString text, QString informativeText, bool isNeedForShowDetailedText) {
    this->EnableToggleButtons(true);
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(text);
    msgBox.setInformativeText(informativeText);
    if (isNeedForShowDetailedText) {
        msgBox.setDetailedText(reply->errorString());
    }
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
                      "<ul> <li>lingerie</li><li>swimwear</li><li>in sexy poses<li> <li>etc.</li></ul>";

        ui->infoBox->setToolTip(tip);

        QToolTip::showText(ui->infoBox->mapToGlobal(ui->infoBox->rect().center()), tip, ui->infoBox, QRect(), 2000);
    }
    else if (selectedIndex == 2) {
        QString tip = "<b>Adult content: </b> Includes nudity or highly sensitive material, NSFW.";

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
        Fetch(1);
    }
}

void MainWindow::Fetch(int add) {
    this->EnableToggleButtons(false);
    m_CurPage = m_CurPage + add;
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

    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, true);

    QNetworkReply *reply = this->Manager->get(request);
    QShortcut *escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), reply); // 'reply' is the parent!
    escShortcut->setContext(Qt::ApplicationShortcut);

    connect(escShortcut, &QShortcut::activated, reply, [reply]() {
        qDebug() << "esc detected";
        if (reply->isRunning()) {
            reply->abort();
            qDebug() << "reply aborted";
        }
    });

    connect(reply, &QNetworkReply::finished, this, &MainWindow::onFetchFinished);
}

void MainWindow::onFetchFinished() {
    qDebug() << "is loading";
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    if (reply->error() == QNetworkReply::OperationCanceledError) {
        qDebug() << "da ngat ket noi mang";
        this->EnableToggleButtons(true);
        reply->deleteLater();
        return;
    }
    if (reply->error() == QNetworkReply::NoError){
        qDebug() << " ko có lỗi mạng";
        qDebug() << "m_AttemptLoading: " + QString::number(this->m_AttemptLoading);

        QByteArray responseData = reply->readAll();
        qDebug() << responseData;
        QJsonDocument JsonReturn = QJsonDocument::fromJson(responseData);

        if (JsonReturn.isArray()) {
            QJsonArray JsonArray = JsonReturn.array();
            qDebug() << JsonArray.isEmpty();
            if (!JsonArray.isEmpty()) {
                this->m_AttemptLoading = 0;
                int randomIndex = QRandomGenerator::global()->bounded(JsonArray.size());

                QJsonObject obj = JsonArray.at(randomIndex).toObject();
                if (obj.value("jpeg_url").toString().isEmpty() == false) {
                    QString fullUrl = obj.value("jpeg_url").toString(); //full img
                    qDebug() << "jpeg_url: " << fullUrl;
                    this->DownloadFullImage(fullUrl);
                }
            }
            else if (m_AttemptLoading == 0) {
                this->m_CurPage = 1;
                Fetch(0);
                this->m_AttemptLoading = 1;
            }
            else if (m_AttemptLoading == 1) {
                qDebug() << "không tải đc do tag";
                this->HandleErrorAndFetchAgain(reply,tr("An error has occured."), tr("Make sure that your selected tags is relatable to eachother, ..."), false);
            }
        }
        else {
            qDebug() << "có lỗi mạng@!!!";
            this->HandleErrorAndFetchAgain(reply, tr("An error has occured."), tr("Please try again few more times. Check if your network connection is okay."), true);
        }
        reply->deleteLater();
    }
}
void MainWindow::DownloadPreviewImage(QString url) {

}

void MainWindow::DownloadFullImage(QString url) {
    QFileInfo fileInfo(url);
    this->m_fullImageExtension = fileInfo.suffix();
    QNetworkRequest request((QUrl(url)));
    request.setTransferTimeout(3000);
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
            this->HandleErrorAndFetchAgain(reply, tr("An error has occured when we tried to download image."), tr("Please try again few more times."), true);
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
    idk.DownloadWallpaper(this->m_fullImageData,this->m_fullImageExtension,config.isStoreCache, config.pathCache);
}


