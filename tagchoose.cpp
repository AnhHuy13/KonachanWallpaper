#include "tagchoose.h"
#include "ui_tagchoose.h"
#include "AppData.h"
#include "checkimages.h"

TagChoose::TagChoose(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TagChoose)
{
    ui->setupUi(this);
    this->setFocus();
    this->setFixedSize(this->size().width(), this->size().height());
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowTitle("Choose tags [*]");
    this->setWindowModified(false);
    Manager = new QNetworkAccessManager(this);

    QShortcut *shortcut = new QShortcut(QKeySequence(Qt::Key_Return), ui->searchBox);
    connect(shortcut, &QShortcut::activated, this, &TagChoose::on_searchBox_returnPressed);
    connect(ui->searchBtn, &QPushButton::clicked, this, &TagChoose::on_searchBox_returnPressed);

    modelListTagsHave = new QStandardItemModel(this);
    modelTagsHaveChosen = new QStandardItemModel(this);
    ui->listView->setModel(modelListTagsHave);
    ui->tagChosenMenu->setModel(modelTagsHaveChosen);
    connect(modelListTagsHave, &QStandardItemModel::itemChanged, this, &TagChoose::onItemChanged);
    ui->tagChosenMenu->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tagChosenMenu, &QListView::customContextMenuRequested, this, &TagChoose::onListContextMenuRequested);

    ui->listView->setUniformItemSizes(true);
    ui->listView->setLayoutMode(QListView::Batched);
    ui->listView->setBatchSize(50);

    lastInput = "";



}


TagChoose::~TagChoose()
{
    delete ui;
}

void TagChoose::EnableBtnState(bool State) {
    ui->searchBtn->setEnabled(State);
    ui->searchBox->setEnabled(State);
}

void TagChoose::GetApi(QString SearchTag) {
    lastInput = ui->searchBox->text();
    this->EnableBtnState(false);
    ui->stackedWidget->setCurrentIndex(0);
    ui->progressBar->setRange(0, 0);

    int limit = 1500;

    QUrl APIUrl;
    if (SearchTag.isEmpty()) {
        APIUrl = QUrl(QString("https://konachan.net/tag.json?order=count&limit=%1").arg(limit));
        qDebug() << APIUrl;
    }
    else {
        APIUrl = QUrl(QString("https://konachan.net/tag.json?name=*%1*&limit=1000&order=count").arg(SearchTag));
        qDebug() << APIUrl;
    }

    QNetworkRequest request(APIUrl);
    QNetworkReply *reply = this->Manager->get(request);

    connect(reply, &QNetworkReply::finished, this, &TagChoose::onSearchFinished);
}

void TagChoose::onSearchFinished() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    UpdateText();

    if (reply->error() == QNetworkReply::NoError){
        QString Response = reply->readAll();
        QJsonDocument JsonReturn = QJsonDocument::fromJson(Response.toUtf8());

        if (JsonReturn.isArray() && !JsonReturn.array().isEmpty()) {

            QJsonArray JsonArray = JsonReturn.array();
            qDebug() << JsonArray.size();

            ui->listView->setUpdatesEnabled(false);

            modelListTagsHave->setRowCount(JsonArray.size());

            QStringList tagsList = AppData::instance().TagsSelected;
            QSet<QString> selectedTagsSet = QSet<QString>(tagsList.begin(), tagsList.end());

            modelListTagsHave->blockSignals(true);

            for (int i = 0; i < JsonArray.size(); i++) {
                QJsonObject tagObject = JsonArray.at(i).toObject();
                QString tagName = tagObject.value("name").toString();

                QStandardItem *it = modelListTagsHave->item(i);

                if (it == nullptr) {
                    it = new QStandardItem();
                    it->setCheckable(true);
                    it->setText(tagName);

                    /* nếu tag đã chọn có chứa tagName thì để thành đã chọn, còn không thì để không */
                    if (selectedTagsSet.contains(tagName)) {
                        it->setCheckState(Qt::Checked);
                    } else {
                        it->setCheckState(Qt::Unchecked);
                    }

                    modelListTagsHave->setItem(i, 0, it);
                }
                else {
                    if (it->text() != tagName) {
                        it->setText(tagName);
                    }

                    if (selectedTagsSet.contains(tagName)) {
                        if (it->checkState() != Qt::Checked) {
                            it->setCheckState(Qt::Checked);
                        }
                    } else {
                        if (it->checkState() != Qt::Unchecked) {
                            it->setCheckState(Qt::Unchecked);
                        }
                    }
                }
            }

            qDebug() << "Successfully load " << modelListTagsHave->rowCount() << " items";
            this->EnableBtnState(true);
            ui->stackedWidget->setCurrentIndex(1);

            modelListTagsHave->blockSignals(false);

            ui->listView->setUpdatesEnabled(true);

            ui->listView->viewport()->update();
        }

        else {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setText("An error has occured.");
            msgBox.setInformativeText("Please try again few more times.");

            QString errorDetail = QString("Error code: %1\nError string: %2")
                                  .arg(reply->error())
                                  .arg(reply->errorString());

            msgBox.setDetailedText(errorDetail);
            QPushButton* okBtn = msgBox.addButton(tr("Try Again"),QMessageBox::ActionRole);
            QPushButton* cancelBtn = msgBox.addButton(tr("Cancel"),QMessageBox::ActionRole);

            msgBox.exec();
            if (msgBox.clickedButton() == okBtn) {
                msgBox.QMessageBox::close();
                this->GetApi(ui->searchBox->text());
            }
            else if (msgBox.clickedButton() == cancelBtn)  {
                msgBox.QMessageBox::close();
                this->close();
            }


            ui->stackedWidget->setCurrentIndex(1);

            qDebug() << "Error :" << reply->errorString();
        }

        reply->deleteLater();
    }
}
void TagChoose::UpdateText() {
    modelTagsHaveChosen->clear();
    QStringList &selectedTags = AppData::instance().TagsSelected;
    for (const QString &tagName : selectedTags) {
        modelTagsHaveChosen->appendRow(new QStandardItem(tagName));
    }
    modelTagsHaveChosen->layoutChanged();
}

void TagChoose::on_searchBox_returnPressed()
{
    /*  konachan xài tag kiểu như "hatsune_miku" chứ ko phải "hatsune miku"
        nên thành ra là tự đổi từ khoảng trắng thành dấu "_" là đc. khỏi cần ngdung sửa lại
        cũng như simplified cho bỏ khoảng trắng thừa
    */
    QString input = ui->searchBox->text().simplified();
    if (lastInput.simplified() != input) {
        input.replace(" ","_");
        ui->searchBox->setText(input);
        GetApi((input));
    }
}
void TagChoose::onListContextMenuRequested(const QPoint &pos) {
    QListView *senderView = qobject_cast<QListView*>(sender());
    if (!senderView) return;

    QModelIndex index = senderView->indexAt(pos);
    if (!index.isValid()) return;

    QMenu menu(this);
    QAction *removeAction = menu.addAction("Remove this tag");

    connect(removeAction, &QAction::triggered, this, [this, index, senderView]() {
        QStandardItem *item = nullptr;

        if (senderView == ui->tagChosenMenu) {
            item = modelTagsHaveChosen->itemFromIndex(index);
        }
        else {
            item = modelListTagsHave->itemFromIndex(index);
        }

        if (item) {
            QString name = item->text();
            qDebug() << "xoá tag:" << name;

            AppData::instance().TagsSelected.removeOne(name);

            QList<QStandardItem*> itemsListTagHaveChose = modelTagsHaveChosen->findItems(name, Qt::MatchExactly);
            for (int i = 0; i < itemsListTagHaveChose.size(); ++i) {
                QStandardItem* it = itemsListTagHaveChose.at(i);
                modelTagsHaveChosen->removeRow(it->row());
            }

            QList<QStandardItem*> itemsListTagAvaiable = modelListTagsHave->findItems(name, Qt::MatchExactly);
            for (int i = 0; i < itemsListTagAvaiable.size(); ++i) {
                QStandardItem* it = itemsListTagAvaiable.at(i);
                it->setCheckState(Qt::Unchecked);
            }
        }
        else {
            qDebug() << "item=nullptr";
        }
    });

    menu.exec(senderView->mapToGlobal(pos));
}

void TagChoose::onItemChanged(QStandardItem *item) {
    QSet<QString> selectedTagsSet = QSet<QString>(AppData::instance().TagsSelected.begin(), AppData::instance().TagsSelected.end());

    if (!item) return;
    this->setWindowModified(true);
    qDebug() << "thay đổi: hàng:" << item->row() << "col" << item->column() << "State:" << item->checkState();

    QString tagName = item->text();

    if (item->checkState() == Qt::Checked) {
        if (!selectedTagsSet.contains(tagName)) {
            AppData::instance().TagsSelected.append(tagName);
        }
    }
    else {
        AppData::instance().TagsSelected.removeOne(tagName);
    }
    UpdateText();
}

void TagChoose::on_buttonBox_accepted()
{
    this->done(QDialog::Accepted);
    emit tagsUpdated();
}

void TagChoose::on_buttonBox_rejected()
{
    this->close();
}

void TagChoose::closeEvent(QCloseEvent *event)
{
    if (AppData::instance().TagsSelected.isEmpty() || AppData::instance().TagsSelected == AppData::instance().lastTagsSelected) {
        event->accept();
        this->setWindowModified(false);
        emit tagsUpdated();
        this->done(QDialog::Accepted);
        return;
    }
    else {
        QMessageBox msgBox;
        msgBox.setText("You have chosen your tags.");
        msgBox.setInformativeText("Do you want to save your tags?");

        QPushButton* noSaveBtn = msgBox.addButton(tr("Don't Save"), QMessageBox::ActionRole);
        QPushButton* cancelBtn = msgBox.addButton(tr("Cancel"), QMessageBox::RejectRole);
        QPushButton* saveBtn = msgBox.addButton(tr("Save"), QMessageBox::AcceptRole);

        msgBox.setDefaultButton(saveBtn);
        msgBox.exec();

        if (msgBox.clickedButton() == noSaveBtn) {
            AppData::instance().TagsSelected = AppData::instance().lastTagsSelected;
            event->accept();
            emit tagsUpdated();
            this->done(QDialog::Accepted);
        }
        else if (msgBox.clickedButton() == saveBtn) {
            event->accept();
            emit tagsUpdated();
            this->done(QDialog::Accepted);
        }
        else if (msgBox.clickedButton() == cancelBtn){
            //bấm x cũng tính luôn
            event->ignore();
        }
    }
}
