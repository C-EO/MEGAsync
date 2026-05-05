#ifndef PASTEMEGALINKSDIALOG_H
#define PASTEMEGALINKSDIALOG_H

#include <QDialog>
#include <QRegularExpression>

namespace Ui {
class PasteMegaLinksDialog;
}

class PasteMegaLinksDialog : public QDialog
{
    Q_OBJECT

public:
    static const int FILE_LINK_SIZE = 54;
    static const int NEW_FILE_LINK_SIZE = 57;

    static const int FOLDER_LINK_SIZE = 34;
    static const int NEW_FOLDER_LINK_SIZE = 38;

    static const int FOLDER_LINK_WITH_SUBFOLDER_SIZE = 43;
    static const int NEW_FOLDER_LINK_WITH_SUBFOLDER_SIZE = 54;

    static const int FOLDER_LINK_WITH_FILE_SIZE = 43;
    static const int NEW_FOLDER_LINK_WITH_FILE_SIZE = 52;

    static const int NEW_COLLECTION_LINK_SIZE = 42;


    QString base64regExp = QString::fromUtf8("A-Za-z0-9_-");

    QRegularExpression rxHeaderFile =
        QRegularExpression(QString::fromUtf8("^#![%1]{8}![%1]{43}").arg(base64regExp));
    QRegularExpression rxHeaderFileNew =
        QRegularExpression(QString::fromUtf8("^file\\/[%1]{8}#[%1]{43}").arg(base64regExp));

    QRegularExpression rxHeaderFolder =
        QRegularExpression(QString::fromUtf8("^#F![%1]{8}![%1]{22}").arg(base64regExp));
    QRegularExpression rxHeaderFolderNew =
        QRegularExpression(QString::fromUtf8("^folder\\/[%1]{8}#[%1]{22}").arg(base64regExp));

    QRegularExpression rxHeaderFolderSubfolder =
        QRegularExpression(QString::fromUtf8("^#F![%1]{8}![%1]{22}![%1]{8}").arg(base64regExp));
    QRegularExpression rxHeaderFolderSubfolderNew = QRegularExpression(
        QString::fromUtf8("^folder\\/[%1]{8}#[%1]{22}\\/folder\\/[%1]{8}").arg(base64regExp));

    QRegularExpression rxHeaderFolderFile =
        QRegularExpression(QString::fromUtf8("^#F![%1]{8}![%1]{22}\\?[%1]{8}").arg(base64regExp));
    QRegularExpression rxHeaderFolderFileNew = QRegularExpression(
        QString::fromUtf8("^folder\\/[%1]{8}#[%1]{22}\\/file\\/[%1]{8}").arg(base64regExp));

    QRegularExpression rxHeaderCollectionNew =
        QRegularExpression(QString::fromUtf8("^collection\\/[%1]{8}#[%1]{22}").arg(base64regExp));

    explicit PasteMegaLinksDialog(QWidget *parent = 0);
    ~PasteMegaLinksDialog();
    QStringList getLinks();

private slots:
    void on_bSubmit_clicked();

protected:
    bool event(QEvent* event);

private:
    Ui::PasteMegaLinksDialog *ui;
    QStringList links;

    QStringList extractLinks(const QString& text);
    QString checkLink(QString link);
};

#endif // PASTEMEGALINKSDIALOG_H
