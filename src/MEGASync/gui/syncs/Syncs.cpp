#include "Syncs.h"

#include "ChooseFolder.h"
#include "mega/types.h"
#include "megaapi.h"
#include "MegaApplication.h"
#include "RequestListenerManager.h"
#include "SyncsData.h"
#include "TextDecorator.h"

Syncs::Syncs(QObject* parent):
    QObject(parent),
    mSyncsData(std::make_unique<SyncsData>()),
    mMegaApi(MegaSyncApp->getMegaApi()),
    mSyncController(SyncController::instance())
{
    connect(&mSyncController, &SyncController::syncAddStatus, this, &Syncs::onSyncAddRequestStatus);
    connect(SyncInfo::instance(), &SyncInfo::syncRemoved, this, &Syncs::onSyncRemoved);
    connect(MegaSyncApp, &MegaApplication::languageChanged, this, &Syncs::onLanguageChanged);

    updateDefaultFolders();
}

void Syncs::addSync(const QString& localFolder, const QString& megaFolder)
{
    cleanErrors();

    mSyncConfig.localFolder = localFolder;
    mSyncConfig.remoteFolder = megaFolder;

    if (checkErrorsOnSyncPaths(mSyncConfig.localFolder, mSyncConfig.remoteFolder))
    {
        emit mSyncsData->syncSetupFailed();

        return;
    }

    auto remoteHandle = mega::INVALID_HANDLE;
    auto megaNode = std::unique_ptr<mega::MegaNode>(
        mMegaApi->getNodeByPath(mSyncConfig.remoteFolder.toUtf8().constData()));
    if (megaNode != nullptr)
    {
        remoteHandle = megaNode->getHandle();
    }

    mSyncConfig.origin = mSyncsData->mSyncOrigin;
    mSyncConfig.remoteHandle = remoteHandle;

    if (remoteHandle == mega::INVALID_HANDLE)
    {
        if (auto rootNode = MegaSyncApp->getRootNode())
        {
            mCreatingFolder = true;

            /*
             *  need to remove the first / from the remote path,
             *  we already state in createFolder the origin point.
             */
            if (mSyncConfig.remoteFolder.indexOf(QLatin1Char('/')) == 0)
            {
                mSyncConfig.remoteFolder.remove(0, 1);
            }

            auto listener =
                RequestListenerManager::instance().registerAndGetFinishListener(this, true);
            mMegaApi->createFolder(mSyncConfig.remoteFolder.toUtf8().constData(),
                                   rootNode.get(),
                                   listener.get());
        }
        else
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR,
                               "Sync creation failed: Root node invalid.");
        }
    }
    else
    {
        mSyncController.addSync(mSyncConfig);
    }
}

void Syncs::setDefaultLocalFolder()
{
    ChooseLocalFolder localFolderChooser;

    QString defaultFolder = localFolderChooser.getDefaultFolder(getDefaultMegaFolder());

    if (mSyncsData->mSyncOrigin != SyncInfo::SyncOrigin::ONBOARDING_ORIGIN &&
        !mRemoteFolder.isEmpty())
    {
        defaultFolder.clear();
    }

    if (helperCheckLocalSync(defaultFolder))
    {
        defaultFolder.clear();
        clearLocalError();
    }

    mSyncsData->mDefaultLocalFolder = defaultFolder;
    emit mSyncsData->defaultLocalFolderChanged();
}

void Syncs::setDefaultRemoteFolder()
{
    QString defaultFolder = getDefaultMegaPath();

    if (mSyncsData->mSyncOrigin != SyncInfo::SyncOrigin::ONBOARDING_ORIGIN &&
        !mRemoteFolder.isEmpty())
    {
        defaultFolder = mRemoteFolder;
    }

    if (helperCheckRemoteSync(defaultFolder))
    {
        defaultFolder.clear();
        clearRemoteError();
    }

    mSyncsData->mDefaultRemoteFolder = defaultFolder;
    emit mSyncsData->defaultRemoteFolderChanged();
}

void Syncs::setSyncOrigin(SyncInfo::SyncOrigin origin)
{
    if (mSyncsData->mSyncOrigin != origin)
    {
        mSyncsData->mSyncOrigin = origin;

        updateDefaultFolders();

        emit mSyncsData->syncOriginChanged();
    }
}

void Syncs::setRemoteFolder(const QString& remoteFolder)
{
    mRemoteFolder = remoteFolder;

    updateDefaultFolders();
}

bool Syncs::checkErrorsOnSyncPaths(const QString& localPath, const QString& remotePath)
{
    checkLocalSync(localPath);
    checkRemoteSync(remotePath);

    return (mLocalError.has_value() || mRemoteError.has_value());
}

QString Syncs::getDefaultMegaFolder()
{
    return DEFAULT_MEGA_FOLDER;
}

QString Syncs::getDefaultMegaPath()
{
    return DEFAULT_MEGA_PATH;
}

std::optional<Syncs::LocalErrors> Syncs::helperCheckLocalSync(const QString& path)
{
    std::optional<LocalErrors> localError;

    if (path.isEmpty())
    {
        localError = LocalErrors::EMPTY_PATH;
    }
    else
    {
        auto localFolderPath = QDir::toNativeSeparators(path);
        QDir openFromFolderDir(localFolderPath);
        if (!openFromFolderDir.exists())
        {
            ChooseLocalFolder localFolder;
            if (localFolderPath == localFolder.getDefaultFolder(DEFAULT_MEGA_FOLDER))
            {
                if (!localFolder.createFolder(localFolderPath))
                {
                    localError = LocalErrors::NO_ACCESS_PERMISSIONS_CANT_CREATE;
                }
            }
            else
            {
                localError = LocalErrors::NO_ACCESS_PERMISSIONS_NO_EXIST;
            }
        }
    }

    if (!localError.has_value())
    {
        QString errorMessage;
        auto syncability =
            mSyncController.isLocalFolderSyncable(path, mega::MegaSync::TYPE_TWOWAY, errorMessage);
        if (syncability == SyncController::CANT_SYNC)
        {
            localError = LocalErrors::CANT_SYNC;
        }
    }

    return localError;
}

std::optional<Syncs::RemoteErrors> Syncs::helperCheckRemoteSync(const QString& path)
{
    std::optional<RemoteErrors> remoteError;

    if (path.isEmpty())
    {
        remoteError = RemoteErrors::EMPTY_PATH;
    }
    else
    {
        auto megaNode = std::unique_ptr<mega::MegaNode>(mMegaApi->getNodeByPath(path.toStdString().c_str()));
        if (megaNode)
        {
            std::unique_ptr<mega::MegaError> remoteMegaError(MegaSyncApp->getMegaApi()->isNodeSyncableWithError(megaNode.get()));
            if (remoteMegaError->getErrorCode() != mega::MegaError::API_OK)
            {
                remoteError = RemoteErrors::CANT_SYNC;
                mRemoteMegaError.error = remoteMegaError->getErrorCode();
                mRemoteMegaError.syncError = remoteMegaError->getSyncError();
            }
        }
        else if (path != getDefaultMegaPath())
        {
            remoteError = RemoteErrors::CANT_SYNC;
        }
    }

    return remoteError;
}

bool Syncs::checkLocalSync(const QString& path)
{
    auto localError = helperCheckLocalSync(path);

    if (mLocalError != localError)
    {
        mLocalError.swap(localError);
        mSyncsData->setLocalError(getLocalError());
    }

    return (!mLocalError.has_value());
}

bool Syncs::checkRemoteSync(const QString& path)
{
    auto remoteError = helperCheckRemoteSync(path);

    if (mRemoteError != remoteError)
    {
        mRemoteError.swap(remoteError);
        mSyncsData->setRemoteError(getRemoteError());
    }

    return (!mRemoteError.has_value());
}

void Syncs::onRequestFinish(mega::MegaRequest* request, mega::MegaError* error)
{
    if (request->getType() == mega::MegaRequest::TYPE_CREATE_FOLDER && mCreatingFolder &&
        (mSyncConfig.remoteFolder.compare(QString::fromUtf8(request->getName())) == 0))
    {
        mCreatingFolder = false;

        if (error->getErrorCode() == mega::MegaError::API_OK)
        {
            auto megaNode = std::shared_ptr<mega::MegaNode>(
                mMegaApi->getNodeByPath(mSyncConfig.remoteFolder.toUtf8().constData(),
                                        MegaSyncApp->getRootNode().get()));
            if (megaNode != nullptr)
            {
                mSyncConfig.remoteHandle = request->getNodeHandle();
                directoryCreatedNextTask();
            }
            else
            {
                mRemoteError = RemoteErrors::CANT_CREATE_REMOTE_FOLDER;

                mSyncsData->setRemoteError(getRemoteError());
            }
        }
        else if (error->getErrorCode() != mega::MegaError::API_ESSL
                && error->getErrorCode() != mega::MegaError::API_ESID)
        {
            mRemoteError = RemoteErrors::CANT_CREATE_REMOTE_FOLDER_MSG;
            mRemoteMegaError.error = mega::MegaError::API_OK;
            mRemoteMegaError.syncError = mega::SyncError::NO_SYNC_ERROR;
            mRemoteStringMessage = QString::fromUtf8(error->getErrorString());

            mSyncsData->setRemoteError(getRemoteError());
        }
    }
}

void Syncs::directoryCreatedNextTask()
{
    mSyncController.addSync(mSyncConfig);
}

void Syncs::onSyncRemoved(std::shared_ptr<SyncSettings> syncSettings)
{
    Q_UNUSED(syncSettings)

    updateDefaultFolders();

    emit mSyncsData->syncRemoved();
}

void Syncs::updateDefaultFolders()
{
    setDefaultLocalFolder();
    setDefaultRemoteFolder();
}

bool Syncs::setErrorIfExist(int errorCode, int syncErrorCode)
{
    if (errorCode != mega::MegaError::API_OK)
    {
        mRemoteError = RemoteErrors::CANT_ADD_SYNC;
        mRemoteMegaError.error = errorCode;
        mRemoteMegaError.syncError = syncErrorCode;

        mSyncsData->setRemoteError(getRemoteError());

        return true;
    }

    return false;
}

void Syncs::onSyncAddRequestStatus(int errorCode, int syncErrorCode, QString name)
{
    Q_UNUSED(name)

    if (!setErrorIfExist(errorCode, syncErrorCode))
    {
        updateDefaultFolders();

        emit mSyncsData->syncSetupSuccess(mSyncConfig.remoteFolder == FULL_SYNC_PATH);
    }
    else
    {
        emit mSyncsData->syncSetupFailed();
    }
}

QString Syncs::getLocalError() const
{
    if (!mLocalError.has_value())
    {
        return {};
    }

    switch (mLocalError.value())
    {
        case LocalErrors::EMPTY_PATH:
        {
            return tr("Select a local folder to sync.");
        }

        case LocalErrors::ALREADY_SYNC_CANDIDATE:
        {
            return getAlreadySyncCandidateError();
        }

        case LocalErrors::NO_ACCESS_PERMISSIONS_CANT_CREATE:
        {
            return QCoreApplication::translate(
                "OnboardingStrings",
                "Folder can’t be synced as you don’t have permissions to create a new folder. To "
                "continue, select an existing folder.");
        }

        case LocalErrors::NO_ACCESS_PERMISSIONS_NO_EXIST:
        {
            return QCoreApplication::translate("MegaSyncError", "Local path not available");
        }

        case LocalErrors::CANT_SYNC:
        {
            QString errorMessage;
            mSyncController.isLocalFolderSyncable(mSyncConfig.localFolder,
                                                  mega::MegaSync::TYPE_TWOWAY,
                                                  errorMessage);
            return errorMessage;
        }
    }

    return {};
}

QString Syncs::getAlreadySyncCandidateError() const
{
    return tr("Folder can't be synced as it's already a candidate.");
}

QString Syncs::getRemoteError() const
{
    if (!mRemoteError.has_value())
    {
        return {};
    }

    switch (mRemoteError.value())
    {
        case RemoteErrors::EMPTY_PATH:
        {
            return tr("Select a MEGA folder to sync.");
        }

        case RemoteErrors::ALREADY_SYNC_CANDIDATE:
        {
            return getAlreadySyncCandidateError();
        }

        case RemoteErrors::CANT_SYNC:
        {
            if (mRemoteMegaError.error != mega::MegaError::API_OK)
            {
                return mSyncController.getRemoteFolderErrorMessage(mRemoteMegaError.error,
                                                                   mRemoteMegaError.syncError);
            }
            else
            {
                return tr("Folder can't be synced as it can't be located. "
                          "It may have been moved or deleted, or you might not have access.");
            }
        }

        case RemoteErrors::CANT_CREATE_REMOTE_FOLDER:
        {
            return tr("%1 folder doesn't exist").arg(mSyncConfig.remoteFolder);
        }

        case RemoteErrors::CANT_CREATE_REMOTE_FOLDER_MSG:
        {
            if (!mRemoteStringMessage.isEmpty())
            {
                return QCoreApplication::translate("MegaError", mRemoteStringMessage.toStdString().c_str());
            }

            break;
        }

        case RemoteErrors::CANT_ADD_SYNC:
        {
            Text::Link link(Utilities::SUPPORT_URL);
            Text::Decorator dec(&link);
            QString msg =
                mSyncController.getErrorString(mRemoteMegaError.error, mRemoteMegaError.syncError);
            dec.process(msg);

            return msg;
        }
    }

    return {};
}

void Syncs::cleanErrors()
{
    clearLocalError();
    clearRemoteError();
}

void Syncs::clearRemoteError()
{
    mRemoteError.reset();
    mRemoteStringMessage.clear();
    mRemoteMegaError.error = mega::MegaError::API_OK;
    mRemoteMegaError.syncError = mega::SyncError::NO_SYNC_ERROR;

    mSyncsData->setRemoteError({});
}

void Syncs::clearLocalError()
{
    mLocalError.reset();
    mSyncsData->setLocalError({});
}

SyncsData* Syncs::getSyncsData() const
{
    return mSyncsData.get();
}

void Syncs::onLanguageChanged()
{
    mSyncsData->setLocalError(getLocalError());
    mSyncsData->setRemoteError(getRemoteError());
}
