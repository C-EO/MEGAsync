#ifndef STALLEDISSUESDIALOG_H
#define STALLEDISSUESDIALOG_H

#include "MegaDelegateHoverManager.h"
#include "StalledIssue.h"
#include "StalledIssueLoadingItem.h"
#include "control/Preferences/Preferences.h"

#include <ViewLoadingScene.h>

#include <QDialog>
#include <QGraphicsDropShadowEffect>

namespace Ui {
class StalledIssuesDialog;
}

class StalledIssueTab;
class StalledIssuesProxyModel;
class StalledIssueDelegate;

class StalledIssuesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit StalledIssuesDialog(QWidget *parent = nullptr);
    ~StalledIssuesDialog();

    QModelIndexList getSelection(QList<mega::MegaSyncStall::SyncStallReason> reasons) const;
    QModelIndexList getSelection(std::function<bool (const std::shared_ptr<const StalledIssue>)> checker) const;

protected:
    bool eventFilter(QObject *, QEvent *) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void changeEvent(QEvent *) override;

private slots:
    void on_doneButton_clicked();
    void on_refreshButton_clicked();
    void checkIfViewIsEmpty();
    void onGlobalSyncStateChanged(bool);

    void toggleTab(StalledIssueFilterCriterion filterCriterion);

    void onUiBlocked();
    void onUiUnblocked();

    void onStalledIssuesLoaded();
    void onModelFiltered();
    void onLoadingSceneVisibilityChange(bool state);

    void showModeSelector();
    void onPreferencesValueChanged(QString key);

private:
    void showView(bool update);
    void selectNewMode();
    void hoverMode(Preferences::StalledIssuesModeType mode);
    void unhoverMode(Preferences::StalledIssuesModeType mode);

    void setLearnMoreLabel();

    bool setNewModeToPreferences();

    Ui::StalledIssuesDialog *ui;
    MegaDelegateHoverManager mViewHoverManager;
    StalledIssueFilterCriterion mCurrentTab;
    StalledIssuesProxyModel* mProxyModel;
    StalledIssueDelegate* mDelegate;

    Preferences::StalledIssuesModeType mModeSelected = Preferences::StalledIssuesModeType::Smart;
};

#endif // STALLEDISSUESDIALOG_H
