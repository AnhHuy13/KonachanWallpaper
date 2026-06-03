#ifndef TAGCHOOSE_H
#define TAGCHOOSE_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QStringList>
#include <QAbstractItemView>
#include <QStandardItem>
#include <QMenu>
#include <QKeyEvent>
#include <QShortcut>
#include <QCloseEvent>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

struct TagItem {
    bool isTicked = false;
    int id;
    int count;
    int type;
};

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

    void EnableBtnState(bool State);

    void CallingSearch();

    void closeEvent(QCloseEvent *event);

private slots:

    void onListContextMenuRequested(const QPoint &pos);

    void deleteSelectedTags(int row);

    void on_searchBox_returnPressed();

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void onItemChanged(QStandardItem *item);

signals :
    void tagsUpdated();

private:
    Ui::TagChoose *ui;
    QNetworkAccessManager *Manager;
    QString lastInput;
    QStandardItemModel *modelTagsHaveChosen;
    QStandardItemModel *modelListTagsHave;
};

#endif // TAGCHOOSE_H
