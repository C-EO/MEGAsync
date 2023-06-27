#ifndef STALLEDISSUEHEADER_H
#define STALLEDISSUEHEADER_H

#include "megaapi.h"
#include "StalledIssue.h"
#include "StalledIssueBaseDelegateWidget.h"

#include <QWidget>
#include <QStyleOptionViewItem>
#include <QFutureWatcher>
#include <QPointer>

#include "ui_StalledIssueHeader.h"

class StalledIssueHeaderCase;

class StalledIssueHeader : public StalledIssueBaseDelegateWidget
{
    Q_OBJECT

public:
    static const int BODY_INDENT;
    static const int ARROW_INDENT;
    static const int ICON_INDENT;
    static const int HEIGHT;
    static const int GROUPBOX_INDENT;
    static const int GROUPBOX_CONTENTS_INDENT;

    explicit StalledIssueHeader(QWidget *parent = nullptr);
    ~StalledIssueHeader();

    void expand(bool state) override;
    virtual bool adaptativeHeight();

    void showAction(const QString& actionButtonText);
    void hideAction();

    void showMessage(const QString& message, const QPixmap &pixmap);

    void setLeftTitleText(const QString& text);
    void addFileName(bool preferCloud = false);
    void addFileName(const QString& filename);
    void setRightTitleText(const QString& text);

    void setTitleDescriptionText(const QString& text);

    void setData(StalledIssueHeaderCase* data);
    void reset();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

    QString fileName();

protected slots:
    virtual void on_actionButton_clicked();
    virtual void on_ignoreFileButton_clicked();

private:
    void showIgnoreFile();
    void issueIgnored();
    void clearLabels();

    void refreshUi() override;

    Ui::StalledIssueHeader *ui;
    QPointer<StalledIssueHeaderCase> mHeaderCase;
};

#endif // STALLEDISSUEHEADER_H
