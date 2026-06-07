#include "BackupSettingsQuickWidget.h"

#include "BackupsController.h"
#include "CreateRemoveBackupsManager.h"
#include "DialogOpener.h"
#include "MessageDialogOpener.h"
#include "MyBackupsHandle.h"
#include "Platform.h"
#include "QmlManager.h"
#include "StatsEventHandler.h"
#include "SyncController.h"
#include "SyncExclusions.h"

#include <QFileInfo>

BackupSettingsQuickWidget::BackupSettingsQuickWidget(QWidget* parent):
    MegaQuickWidget(parent),
    mBackupModel(new BackupSettingsModel(this)),
    mMyBackupsHandleRequest(UserAttributes::MyBackupsHandle::requestMyBackupsHandle()),
    mBackupFolderPath(UserAttributes::MyBackupsHandle::getMyBackupsLocalizedPath()),
    mBackupFolderAvailable(false)
{
    setResizeMode(QQuickWidget::SizeRootObjectToView);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    qmlRegisterType<BackupSettingsModel>("BackupSettingsModel", 1, 0, "BackupSettingsModel");

    QmlManager::instance()->setRootContextProperty(QStringLiteral("backupSettingsModel"),
                                                   mBackupModel);
    QmlManager::instance()->setRootContextProperty(QStringLiteral("backupSettings"), this);

    connect(mMyBackupsHandleRequest.get(),
            &UserAttributes::MyBackupsHandle::attributeReady,
            this,
            &BackupSettingsQuickWidget::onMyBackupsFolderHandleSet);
    onMyBackupsFolderHandleSet(mMyBackupsHandleRequest->getMyBackupsHandle());

    setSource(QString::fromUtf8("qrc:/settings/BackupSettings.qml"));
}

void BackupSettingsQuickWidget::exploreLocalBackup(const QString& localFolder) const
{
    Platform::getInstance()->showInFolder(localFolder);
}

void BackupSettingsQuickWidget::openInMega(int index) const
{
    Utilities::openInMega(mBackupModel->getBackup(index)->getMegaHandle());
}

void BackupSettingsQuickWidget::addBackup() const
{
    CreateRemoveBackupsManager::addBackup(SyncInfo::SyncOrigin::SETTINGS_ORIGIN,
                                          QStringList(),
                                          this->parentWidget());
}

void BackupSettingsQuickWidget::pauseBackup(int index) const
{
    BackupsController::instance().setSyncToSuspend(mBackupModel->getBackup(index));
}

void BackupSettingsQuickWidget::resumeBackup(int index) const
{
    BackupsController::instance().setSyncToRun(mBackupModel->getBackup(index));
}

void BackupSettingsQuickWidget::openExclusionsDialog(int index) const
{
    const auto& backup = mBackupModel->getBackup(index);
    QFileInfo backupDir(backup->getLocalFolder());
    if (backupDir.exists())
    {
        QPointer<QmlDialogWrapper<SyncExclusions>> exclusions =
            new QmlDialogWrapper<SyncExclusions>(this->parentWidget(), backup->getLocalFolder());

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

void BackupSettingsQuickWidget::openOverQuotaDialog() const
{
    auto overQuotaDialog = MegaSyncApp->createOverquotaDialogIfNeeded();

    if (overQuotaDialog)
    {
        DialogOpener::showDialog(overQuotaDialog);
    }
}

void BackupSettingsQuickWidget::openBackupFolder() const
{
    if (!mBackupFolderAvailable)
    {
        return;
    }

    Utilities::openInMega(mMyBackupsHandleRequest->getMyBackupsHandle());
    MegaSyncApp->getStatsEventHandler()->sendTrackedEvent(
        AppStatsEvents::EventType::SETTINGS_VIEW_BACKUP_CLICKED);
}

void BackupSettingsQuickWidget::remove(int index) const
{
    const auto& backup = mBackupModel->getBackup(index);
    CreateRemoveBackupsManager::removeBackup(backup, this->parentWidget());
}

void BackupSettingsQuickWidget::sortModelByName(bool ascending)
{
    mBackupModel->sortByName(ascending);
}

void BackupSettingsQuickWidget::sortModelByStatus(bool ascending)
{
    mBackupModel->sortByStatus(ascending);
}

void BackupSettingsQuickWidget::removeNonConfirmation(int index) const
{
    const auto& backup = mBackupModel->getBackup(index);
    BackupsController::instance().removeSync(backup);
}

void BackupSettingsQuickWidget::rescan(int index) const
{
    const auto& backup = mBackupModel->getBackup(index);
    MegaSyncApp->getMegaApi()->rescanSync(backup->backupId(), true);
}

void BackupSettingsQuickWidget::reboot(int index) const
{
    const auto& backup = mBackupModel->getBackup(index);
    BackupsController::instance().resetSync(backup,
                                            mega::MegaSync::SyncRunningState::RUNSTATE_DISABLED);
}

QString BackupSettingsQuickWidget::getBackupFolderPath() const
{
    return mBackupFolderPath;
}

bool BackupSettingsQuickWidget::isBackupFolderAvailable() const
{
    return mBackupFolderAvailable;
}

void BackupSettingsQuickWidget::onMyBackupsFolderHandleSet(mega::MegaHandle handle)
{
    const auto newPath = UserAttributes::MyBackupsHandle::getMyBackupsLocalizedPath();
    const auto newAvailable = handle != mega::INVALID_HANDLE;

    if (mBackupFolderPath != newPath)
    {
        mBackupFolderPath = newPath;
        emit backupFolderPathChanged();
    }

    if (mBackupFolderAvailable != newAvailable)
    {
        mBackupFolderAvailable = newAvailable;
        emit backupFolderAvailableChanged();
    }
}
