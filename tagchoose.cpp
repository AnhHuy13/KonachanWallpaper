#include "tagchoose.h"
#include "ui_tagchoose.h"
#include <iostream>
#include <vector>
#include <stdio.h>
#include "AppData.h"

TagChoose::TagChoose(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TagChoose)
{
    ui->setupUi(this);
    this->setFocus();
    this->setFixedSize(this->size().width(), this->size().height());
    this->setAttribute(Qt::WA_DeleteOnClose);
    Manager = new QNetworkAccessManager(this);
    ui->tableWidget->viewport()->installEventFilter(this);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->listWidget->setUniformItemSizes(true);
    ui->listWidget->setBatchSize(100);
    connect(ui->listWidget, &QListWidget::customContextMenuRequested,this, &TagChoose::onListContextMenuRequested);
    QShortcut *shortcut = new QShortcut(QKeySequence(Qt::Key_Return), ui->searchBox);
    connect(shortcut, &QShortcut::activated, this, &TagChoose::on_searchBox_returnPressed);

    connect(ui->searchBtn, &QPushButton::clicked, this, &TagChoose::on_searchBox_returnPressed);

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

    QUrl APIUrl;
    if (SearchTag.isEmpty()) {
        APIUrl = QUrl("https://konachan.net/tag.json?order=count&limit=1500");
    }
    else {
        QString urlText = QString("https://konachan.net/tag.json?name=*%1*&limit=1500&order=count").arg(SearchTag);
        APIUrl = QUrl(urlText);
    }

    QNetworkRequest request(APIUrl);
    QNetworkReply *reply = this->Manager->get(request);

    connect(reply, &QNetworkReply::finished, this, &TagChoose::onSearchFinished);
}

void TagChoose::onSearchFinished() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    ui->tableWidget->clearContents();
    UpdateText();

    if (reply->error() == QNetworkReply::NoError){
        QString Response = reply->readAll();
        QJsonDocument JDoc = QJsonDocument::fromJson(Response.toUtf8());
        if (JDoc.isArray()) {
            QJsonArray JArray = JDoc.array();
            qDebug() << JArray.count();
            m_tagList.clear();
            ui->tableWidget->setRowCount(JArray.count());
            ui->tableWidget->setColumnCount(2);

            QStringList tagNames;
            QList<TagItem> tagMetadata;

            ui->tableWidget->blockSignals(true);
            for (int i = 0; i < JArray.count(); i++) {
                QJsonObject tagObject = JArray.at(i).toObject();

                TagItem item;
                item.isTicked = false;
                item.id = tagObject.value("id").toInt();
                item.name = tagObject.value("name").toString();
                item.count = tagObject.value("count").toInt();
                item.type = tagObject.value("type").toInt();

                m_tagList.append(item);
                TagItem curTag = m_tagList.at(i);

                QTableWidgetItem* itemName = new QTableWidgetItem(tagObject.value("name").toString());
                itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
                QTableWidgetItem* itemCheck = new QTableWidgetItem();
                itemCheck->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);

                if (AppData::instance().TagsSelected.contains(tagObject.value("name").toString())) {
                    itemCheck->setCheckState(Qt::Checked);
                } else {
                    itemCheck->setCheckState(Qt::Unchecked);
                }

                itemName->setData(Qt::UserRole + 0, curTag.id);
                itemName->setData(Qt::UserRole + 1, curTag.type);
                itemName->setData(Qt::UserRole + 2, curTag.count);

                ui->tableWidget->setItem(i, 0, itemCheck);
                ui->tableWidget->setItem(i, 1, itemName);
            }
            ui->tableWidget->blockSignals(false);
            qDebug() << "successfully put data to tables";
            this->EnableBtnState(true);
            ui->stackedWidget->setCurrentIndex(1);
            ui->tableWidget->horizontalHeader()->resizeSection(0,40);
            ui->tableWidget->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
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
            msgBox.QMessageBox::reject();
            this->GetApi(ui->searchBox->text()); // try again with the user's search
        }
        else if (msgBox.clickedButton() == cancelBtn)  {
            msgBox.QMessageBox::reject();
            this->close();
        }


        ui->stackedWidget->setCurrentIndex(1);

        qDebug() << "Error :" << reply->errorString();
    }

    reply->deleteLater();
}

void TagChoose::UpdateText() {
    ui->listWidget->clear();
    ui->listWidget->addItems(AppData::instance().TagsSelected);
}

void TagChoose::on_searchBox_returnPressed()
{
    // the konachan tags using tags name like "abc_xyz",... prevent the case user's search has space
    // characters and replace it to underscore then GetApi() it

    QString input = ui->searchBox->text();
    if (lastInput != input) {
        input.replace(" ","_");
        ui->searchBox->setText(input);
        GetApi((input));
    }
}

void TagChoose::on_tableWidget_itemChanged(QTableWidgetItem *item) {
    if (item->column() != 0) return;

    qDebug() << "Item changed! Row:" << item->row() << "column" << item->column() << "State:" << item->checkState();

    ui->tableWidget->blockSignals(true);

    QTableWidgetItem* nameItem = ui->tableWidget->item(item->row(), 1);
    if (nameItem) {
        QString tagName = nameItem->text();

        if (item->checkState() == Qt::Checked) {
            if (!AppData::instance().TagsSelected.contains(tagName)) {
                AppData::instance().TagsSelected.append(tagName);
                qDebug() << "Added tag:" << tagName;
            }
        } else {
            AppData::instance().TagsSelected.removeOne(tagName);
            qDebug() << "Removed tag:" << tagName;
        }
        UpdateText();
    }

    ui->tableWidget->blockSignals(false);
    qDebug() << "Finished processing row:" << item->row() << "column" << item->column() << "State:" << item->checkState();
}

void TagChoose::onListContextMenuRequested(const QPoint &pos) {
    QListWidgetItem *item = ui->listWidget->itemAt(pos);
    if (!item) return;

    QMenu menu(this);
    QAction *deleteAction = menu.addAction("Remove this tag");

    connect(deleteAction, &QAction::triggered, this, &TagChoose::deleteSelectedTag);

    ui->listWidget->setCurrentItem(item);

    menu.exec(ui->listWidget->mapToGlobal(pos));
}

void TagChoose::deleteSelectedTag() {
    QListWidgetItem *list_item = ui->listWidget->currentItem();
    if (!list_item) return;

    QString tagName = list_item->text();

    AppData::instance().TagsSelected.removeOne(tagName);

    auto foundItems = ui->tableWidget->findItems(tagName, Qt::MatchExactly);
    for (auto* item : foundItems) {
        ui->tableWidget->blockSignals(true);
        ui->tableWidget->item(item->row(), 0)->setCheckState(Qt::Unchecked);
        ui->tableWidget->blockSignals(false);
    }

    // update the ui
    delete list_item;
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
            // when user press cancel or X in message box
            event->ignore();
        }
    }
}
