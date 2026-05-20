#include "tagchoose.h"
#include "ui_tagchoose.h"
#include <iostream>
#include <vector>
#include <stdio.h>

QStringList TagsSelected;

TagChoose::TagChoose(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TagChoose)
{
    ui->setupUi(this);
    this->setFocus();
    this->setFixedSize(this->size().width(), this->size().height());

    // Thiết lập CSS để tăng kích thước hitbox của checkbox
    ui->tableWidget->setStyleSheet(
        "QTableWidget::indicator {"
        "    width: 20px;"
        "    height: 20px;"
        "}"
    );
}

TagChoose::~TagChoose()
{
    delete ui;
}

void TagChoose::GetApi(QString SearchTag) {
    QNetworkAccessManager *Manager = new QNetworkAccessManager(this);
    QUrl APIUrl;
    if (SearchTag.isEmpty()) {
        APIUrl = QUrl("https://konachan.net/tag.json?order=count&limit=1500");
    }
    else {
        QString urlText = QString("https://konachan.net/tag.json?name=*%1*&limit=1500&order=count").arg(SearchTag);
        APIUrl = QUrl(urlText);
    }

    QNetworkRequest request(APIUrl);
    QNetworkReply *reply = Manager->get(request);

    connect(reply, &QNetworkReply::finished, this, &TagChoose::onSearchFinished);
}

void TagChoose::onSearchFinished() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (reply->error() == QNetworkReply::NoError){
        QString Response = reply->readAll();
        qDebug() << "Api Response:" << Response;
        QJsonDocument JDoc = QJsonDocument::fromJson(Response.toUtf8());
        if (JDoc.isArray()) {
            QJsonArray JArray = JDoc.array();
            qDebug() << "Tổng số tag cào được:" << JArray.count();
            m_tagList.clear();
            ui->tableWidget->setRowCount(JArray.count());
            ui->tableWidget->setColumnCount(2);
            ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
            ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);


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
                QTableWidgetItem* itemCheck = new QTableWidgetItem();
                itemCheck->setFlags(itemCheck->flags() | Qt::ItemIsUserCheckable);
                itemCheck->setFlags(itemCheck->flags() | Qt::ItemIsEnabled);
                ui->tableWidget->viewport()->installEventFilter(this);
                ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

                if (TagsSelected.contains(tagObject.value("name").toString())) {
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
            qDebug() << "đã lưu thành công!";
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
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();

        qDebug() << "Error :" << reply->errorString();
    }

    reply->deleteLater();
    reply->manager()->deleteLater();
}

void TagChoose::UpdateText() {
    ui->txtChosenTags->setText(TagsSelected.join(", "));
}

void TagChoose::on_searchBtn_clicked()
{
    QString input = ui->searchBox->text();
    input.replace(" ","_");
    GetApi((input));
}

void TagChoose::on_tableWidget_itemClicked(QTableWidgetItem *item)
{
    // Bỏ
}

void TagChoose::on_tableWidget_itemChanged(QTableWidgetItem *item)
{
    // Bỏ
}

void TagChoose::on_tableWidget_cellClicked(int row, int column)
{
    qDebug() << "BẠN ĐÃ BẤM VÀO DÒNG" << row << "CỘT" << column;
    QTableWidgetItem* checkItem = ui->tableWidget->item(row, 0);
    QTableWidgetItem* nameItem = ui->tableWidget->item(row, 1);

    checkItem->setCheckState((checkItem->checkState() == Qt::Checked) ? Qt::Unchecked : Qt::Checked);
    qDebug() << "ĐÃ ĐỔI STATE Ở DÒNG" << row << "CỘT" << column;
    if (!checkItem || !nameItem) return;

    // Đảo trạng thái trực tiếp
    Qt::CheckState next = (checkItem->checkState() == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
    checkItem->setCheckState(next);

    // Cập nhật dữ liệu vào TagsSelected
    if (next == Qt::Checked) {
        if (!TagsSelected.contains(nameItem->text())) TagsSelected.append(nameItem->text());
    } else {
        TagsSelected.removeOne(nameItem->text());
    }

    UpdateText();
}
