#include "SyncSettingsQuickWidget.h"

#include "CreateRemoveSyncsManager.h"
#include "Platform.h"
#include "QmlManager.h"
#include "RequestListenerManager.h"
#include "StalledIssuesModel.h"
#include "StatsEventHandler.h"
#include "SyncController.h"
#include "SyncExclusions.h"
#include "SyncSettingsModel.h"

SyncSettingsQuickWidget::SyncSettingsQuickWidget(QWidget* parent):
    MegaQuickWidget(parent),
    mAutomaticSyncIssueResolverEnabled(Preferences::instance()->isStalledIssueSmartModeActivated()),
    mSyncModel(new SyncSettingsModel(this))
{
    setResizeMode(QQuickWidget::SizeRootObjectToView);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    qmlRegisterType<SyncSettingsModel>("SyncSettingsModel", 1, 0, "SyncSettingsModel");

    QmlManager::instance()->setRootContextProperty(QStringLiteral("syncSettingsModel"), mSyncModel);
    QmlManager::instance()->setRootContextProperty(QStringLiteral("syncSettings"), this);

    setSource(QString::fromUtf8("qrc:/settings/SyncSettings.qml"));
}

void SyncSettingsQuickWidget::exploreLocalSync(const QString& localFolder) const
{
    Platform::getInstance()->showInFolder(localFolder);
}

void SyncSettingsQuickWidget::openInMega(int index) const
{
    Utilities::openInMega(mSyncModel->getSync(index)->getMegaHandle());
}

void SyncSettingsQuickWidget::addSync() const
{
    CreateRemoveSyncsManager::addSync(SyncInfo::SyncOrigin::SETTINGS_ORIGIN);
}

bool SyncSettingsQuickWidget::getAutomaticSyncIssueResolverEnabled() const
{
    return mAutomaticSyncIssueResolverEnabled;
}

void SyncSettingsQuickWidget::pauseSync(int index) const
{
    SyncController::instance().setSyncToSuspend(mSyncModel->getSync(index));
}

void SyncSettingsQuickWidget::resumeSync(int index) const
{
    SyncController::instance().setSyncToRun(mSyncModel->getSync(index));
}

void SyncSettingsQuickWidget::restoreSyncedFolder(int index)
{
    auto sync = mSyncModel->getSync(index);

    auto triggerErrorMessage = [sync, this]()
    {
        QString logMsg = tr("Can't restore %1 mega folder").arg(sync->getMegaFolder());

        MessageDialogInfo msgInfo;
        msgInfo.parent = this->parentWidget();
        msgInfo.descriptionText = logMsg;
        msgInfo.buttons = QMessageBox::Cancel | QMessageBox::Yes;
        QMap<QMessageBox::Button, QString> textsByButton;
        textsByButton.insert(QMessageBox::Yes, tr("Remove sync"));
        textsByButton.insert(
            QMessageBox::Cancel,
            tr("Close")); // :-( , need to add this, so i've the outline style button.
        msgInfo.buttonsText = textsByButton;

        msgInfo.defaultButton = QMessageBox::Close;
        msgInfo.finishFunc = [sync](QPointer<MessageDialogResult> msg)
        {
            if (msg->result() == QMessageBox::Yes)
            {
                SyncController::instance().removeSync(sync);
            }
        };

        MessageDialogOpener::critical(msgInfo);

        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());
    };

    auto node = std::shared_ptr<mega::MegaNode>(
        MegaSyncApp->getMegaApi()->getNodeByHandle(sync->getMegaHandle()));
    if (node)
    {
        auto restoreNode = std::shared_ptr<mega::MegaNode>(
            MegaSyncApp->getMegaApi()->getNodeByHandle(node->getRestoreHandle()));

        if (restoreNode)
        {
            auto listener = RequestListenerManager::instance().registerAndGetCustomFinishListener(
                this,
                [sync, triggerErrorMessage](mega::MegaRequest* request, mega::MegaError* e)
                {
                    int errorCode = e->getErrorCode();

                    if (errorCode != mega::MegaError::API_OK)
                    {
                        triggerErrorMessage();
                    }
                    else
                    {
                        SyncController::instance().setSyncToRun(sync);
                    }
                });

            MegaSyncApp->getMegaApi()->moveNode(node.get(), restoreNode.get(), listener.get());

            return;
        }
    }

    triggerErrorMessage();
}

void SyncSettingsQuickWidget::openOverQuotaDialog() const
{
    auto overQuotaDialog = MegaSyncApp->createOverquotaDialogIfNeeded();

    if (overQuotaDialog)
    {
        DialogOpener::showDialog(overQuotaDialog);
    }
}

void SyncSettingsQuickWidget::openExclusionsDialog(int index) const
{
    const auto& sync = mSyncModel->getSync(index);
    QFileInfo syncDir(sync->getLocalFolder());
    if (syncDir.exists())
    {
        QPointer<QmlDialogWrapper<SyncExclusions>> exclusions =
            new QmlDialogWrapper<SyncExclusions>(this->parentWidget(), sync->getLocalFolder());

        DialogOpener::showDialog(exclusions);
    }
    else
    {
        MessageDialogInfo msgInfo;
        msgInfo.parent = this->parentWidget();
        msgInfo.descriptionText = tr("Error opening megaignore file");
        MessageDialogOpener::warning(msgInfo);
    }
}

void SyncSettingsQuickWidget::remove(int index) const
{
    const auto& sync = mSyncModel->getSync(index);
    CreateRemoveSyncsManager::removeSync(sync, this->parentWidget());
}

void SyncSettingsQuickWidget::sortModelByName(bool ascending)
{
    mSyncModel->sortByName(ascending);
}

void SyncSettingsQuickWidget::sortModelByStatus(bool ascending)
{
    mSyncModel->sortByStatus(ascending);
}

void SyncSettingsQuickWidget::removeNonConfirmation(int index) const
{
    const auto& sync = mSyncModel->getSync(index);
    SyncController::instance().removeSync(sync);
}

void SyncSettingsQuickWidget::rescan(int index) const
{
    const auto& sync = mSyncModel->getSync(index);
    MegaSyncApp->getMegaApi()->rescanSync(sync->backupId(), true);
}

void SyncSettingsQuickWidget::reboot(int index) const
{
    const auto& sync = mSyncModel->getSync(index);
    SyncController::instance().resetSync(sync, mega::MegaSync::SyncRunningState::RUNSTATE_DISABLED);
}

void SyncSettingsQuickWidget::setAutomaticSyncIssueResolverEnabled(bool enable)
{
    if (mAutomaticSyncIssueResolverEnabled != enable)
    {
        if (enable)
        {
            Preferences::instance()->setStalledIssuesMode(
                Preferences::StalledIssuesModeType::Smart);
            MegaSyncApp->getStalledIssuesModel()->updateActiveStalledIssues();
            MegaSyncApp->getStatsEventHandler()->sendEvent(
                AppStatsEvents::EventType::SETTINGS_ISSUE_RESOLUTION_SMART);
        }
        else
        {
            Preferences::instance()->setStalledIssuesMode(
                Preferences::StalledIssuesModeType::Advance);
            MegaSyncApp->getStatsEventHandler()->sendEvent(
                AppStatsEvents::EventType::SETTINGS_ISSUE_RESOLUTION_ADVANCED);
        }

        mAutomaticSyncIssueResolverEnabled = enable;
        emit automaticSyncIssueResolverEnabledChanged();
    }
}