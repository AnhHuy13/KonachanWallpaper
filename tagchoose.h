#ifndef TAGCHOOSE_H
#define TAGCHOOSE_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <stdio.h>
#include <QNetworkReply>
#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QStringList>
#include <QAbstractItemView>

struct TagItem {
    bool isTicked = false;
    int id;
    QString name;
    int count;
    int type;
};

extern QStringList TagsSelected;

namespace Ui {
class TagChoose;
}

Q_DECLARE_METATYPE(TagItem);


class TagChoose : public QDialog
{
    Q_OBJECT

public:
    explicit TagChoose(QWidget *parent = nullptr);
    QVector<TagItem> m_tagList;
    ~TagChoose();
    void GetApi(QString SearchTag = "");
    void onSearchFinished();
    void UpdateText();

private slots:

    void on_searchBtn_clicked();

    void on_tableWidget_itemChanged(QTableWidgetItem *item);

    void on_tableWidget_itemClicked(QTableWidgetItem *item);

    void on_tableWidget_cellClicked(int row, int column);

private:
    Ui::TagChoose *ui;
};
#endif // TAGCHOOSE_H
