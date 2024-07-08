#include "StalledIssue.h"

#include "MegaApplication.h"
#include "UserAttributesRequests/FullName.h"
#include "StalledIssuesUtilities.h"
#include "TransfersModel.h"
#include "MultiStepIssueSolver.h"

StalledIssueData::StalledIssueData()
{
    qRegisterMetaType<StalledIssueDataPtr>("StalledIssueDataPtr");
    qRegisterMetaType<StalledIssuesDataList>("StalledIssuesDataList");

    qRegisterMetaType<StalledIssueVariant>("StalledIssueVariant");
    qRegisterMetaType<StalledIssuesVariantList>("StalledIssuesVariantList");
}

const StalledIssueData::Path& StalledIssueData::getPath() const
{
    return mPath;
}

const StalledIssueData::Path& StalledIssueData::getMovePath() const
{
    return mMovePath;
}

QString StalledIssueData::getFilePath() const
{
    QFileInfo filePath(mPath.path);
    return filePath.filePath();
}

QString StalledIssueData::getMoveFilePath() const
{
    QFileInfo filePath(mMovePath.path);
    return filePath.filePath();
}

QString StalledIssueData::getNativeFilePath() const
{
    QString path;

    if(isCloud())
    {
        path = mPath.path;
    }
    else
    {
        QFileInfo filePath(mPath.path);
        path = QDir::toNativeSeparators(filePath.filePath());
    }

    return path;
}

QString StalledIssueData::getNativeMoveFilePath() const
{
    QString path;

    if(isCloud())
    {
        path = mMovePath.path;
    }
    else
    {
        QFileInfo filePath(mMovePath.path);
        path =  QDir::toNativeSeparators(filePath.filePath());
    }

    return path;
}

QString StalledIssueData::getNativePath() const
{
    QFileInfo filePath(mPath.path);

    if(isCloud())
    {
        return filePath.path();
    }
    else
    {
        return QDir::toNativeSeparators(filePath.path());
    }
}

QString StalledIssueData::getNativeMovePath() const
{
    QFileInfo filePath(mMovePath.path);

    if(isCloud())
    {
        return filePath.path();
    }
    else
    {
        return QDir::toNativeSeparators(filePath.path());
    }
}

QString StalledIssueData::getFileName() const
{
    QFileInfo filePath(getNativeFilePath());
    QString fileName;

    if(filePath.isFile())
    {
        fileName = filePath.fileName();
    }
    else
    {
        if(isCloud())
        {
            auto splittedIndexPath = mPath.path.split(QString::fromUtf8("/"));
            fileName = splittedIndexPath.last();
        }
        else
        {
            auto splittedIndexPath = mPath.path.split(QDir::separator());
            fileName = splittedIndexPath.last();
        }
    }

    checkTrailingSpaces(fileName);

    return fileName;
}

void StalledIssueData::checkTrailingSpaces(QString& name) const
{
    auto trimmedPath = name.trimmed();
    if(trimmedPath != name)
    {
        name.prepend(QString::fromUtf8("\""));
        name.append(QString::fromUtf8("\""));
    }
}

//CLOUD
std::shared_ptr<mega::MegaNode> CloudStalledIssueData::getNode(bool refresh) const
{
    if(refresh)
    {
        mRemoteNode.reset();
    }
    else
    {
        if(mRemoteNode && mRemoteNode->getHandle() == mPathHandle)
        {
            return mRemoteNode;
        }
    }

    auto newNode = std::shared_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getNodeByHandle(mPathHandle));
    if(!newNode)
    {
        return mRemoteNode;
    }
    else
    {
        mRemoteNode = newNode;
        return newNode;
    }
}

mega::MegaHandle CloudStalledIssueData::getPathHandle() const
{
    return mPathHandle;
}

mega::MegaHandle CloudStalledIssueData::getMovePathHandle() const
{
    return mMovePathHandle;
}

void CloudStalledIssueData::setPathHandle(mega::MegaHandle newPathHandle)
{
    mPathHandle = newPathHandle;
    auto attr = getFileFolderAttributes();
    if(attr)
    {
        attr->setHandle(mPathHandle);
    }

}

////////////////////////////////////////////////////////////////////////////////
/// \brief StalledIssue::StalledIssue
/// \param stallIssue
///
StalledIssue::StalledIssue(const mega::MegaSyncStall* stallIssue)
    : mFileSystemWatcher(new FileSystemSignalHandler(this))
    , mAutoResolutionApplied(false)
{
    originalStall.reset(stallIssue->copy());
    fillBasicInfo(stallIssue);
}

bool StalledIssue::initLocalIssue()
{
    if(!mLocalData)
    {
        mLocalData = QExplicitlySharedDataPointer<LocalStalledIssueData>(new LocalStalledIssueData());
        return true;
    }

    return false;
}

bool StalledIssue::initCloudIssue()
{
    if(!mCloudData)
    {
        mCloudData = QExplicitlySharedDataPointer<CloudStalledIssueData>(new CloudStalledIssueData());

        return true;
    }

    return false;
}

void StalledIssue::fillIssue(const mega::MegaSyncStall* stall)
{
    auto localSourcePathProblem = static_cast<mega::MegaSyncStall::SyncPathProblem>(stall->pathProblem(false, 0));
    auto localTargetPathProblem = static_cast<mega::MegaSyncStall::SyncPathProblem>(stall->pathProblem(false, 1));

    auto localSourcePath = QString::fromUtf8(stall->path(false, 0));
    auto localTargetPath = QString::fromUtf8(stall->path(false, 1));

    if(localSourcePathProblem != mega::MegaSyncStall::SyncPathProblem::NoProblem || !localSourcePath.isEmpty())
    {
        initLocalIssue();
        getLocalData()->mPath.path = localSourcePath;
        getLocalData()->mPath.pathProblem = localSourcePathProblem;

        if(stall->couldSuggestIgnoreThisPath(false, 0))
        {
            mIgnoredPaths.append(getLocalData()->getNativeFilePath());
        }

        setIsFile(localSourcePath, true);
    }

    if(localTargetPathProblem != mega::MegaSyncStall::SyncPathProblem::NoProblem || !localTargetPath.isEmpty())
    {
        initLocalIssue();
        getLocalData()->mMovePath.path = localTargetPath;
        getLocalData()->mMovePath.pathProblem = localTargetPathProblem;

        if(stall->couldSuggestIgnoreThisPath(false, 1))
        {
            mIgnoredPaths.append(getLocalData()->getNativeMoveFilePath());
        }

        setIsFile(localTargetPath, true);
    }

    auto cloudSourcePathProblem = static_cast<mega::MegaSyncStall::SyncPathProblem>(stall->pathProblem(true, 0));
    auto cloudTargetPathProblem = static_cast<mega::MegaSyncStall::SyncPathProblem>(stall->pathProblem(true, 1));

    auto cloudSourcePath = QString::fromUtf8(stall->path(true, 0));
    auto cloudTargetPath = QString::fromUtf8(stall->path(true, 1));

    if(cloudSourcePathProblem != mega::MegaSyncStall::SyncPathProblem::NoProblem || !cloudSourcePath.isEmpty())
    {
        initCloudIssue();
        getCloudData()->mPath.path = cloudSourcePath;
        getCloudData()->mPathHandle = stall->cloudNodeHandle(0);
        getCloudData()->mPath.pathProblem = cloudSourcePathProblem;

        if(stall->couldSuggestIgnoreThisPath(true, 0))
        {
            mIgnoredPaths.append(cloudSourcePath);
        }

        setIsFile(cloudSourcePath, false);
    }

    if(cloudTargetPathProblem != mega::MegaSyncStall::SyncPathProblem::NoProblem || !cloudTargetPath.isEmpty())
    {
        initCloudIssue();
        getCloudData()->mMovePath.path = cloudTargetPath;
        getCloudData()->mMovePathHandle = stall->cloudNodeHandle(1);
        getCloudData()->mMovePath.pathProblem = cloudTargetPathProblem;

        if(stall->couldSuggestIgnoreThisPath(true, 1))
        {
            mIgnoredPaths.append(cloudTargetPath);
        }

        setIsFile(cloudTargetPath, false);
    }

    if(missingFingerprint())
    {
        std::shared_ptr<DownloadTransferInfo> info(new DownloadTransferInfo());
        //Check if transfer already exists
        if(isBeingSolvedByDownload(info))
        {
            setIsSolved(StalledIssue::SolveType::SOLVED);
        }
    }
}

void StalledIssue::fillBasicInfo(const mega::MegaSyncStall* stall)
{
    mReason = stall->reason();
    mDetectedMEGASide = stall->detectedCloudSide();

    mSyncIds = StalledIssuesBySyncFilter::getSyncIdsByStall(stall);
}

void StalledIssue::endFillingIssue()
{
    //Fill user info
    if(mCloudData)
    {
       mCloudData->initFileFolderAttributes();
    }

    if(mLocalData)
    {
        mLocalData->initFileFolderAttributes();
    }

    mNeedsUIUpdate = qMakePair(true, true);
}

const QSet<mega::MegaHandle>& StalledIssue::syncIds() const
{
    return mSyncIds;
}

mega::MegaSync::SyncType StalledIssue::getSyncType() const
{
    mega::MegaSync::SyncType type(mega::MegaSync::SyncType::TYPE_UNKNOWN);
    if(!syncIds().isEmpty())
    {
        foreach(auto& syncId, syncIds())
        {
            auto sync = SyncInfo::instance()->getSyncSettingByTag(syncId);
            if(sync)
            {
                auto syncType = sync->getType();
                if(type != mega::MegaSync::SyncType::TYPE_UNKNOWN &&
                   type != syncType)
                {
                    type = mega::MegaSync::SyncType::TYPE_UNKNOWN;
                    break;
                }

                type = syncType;
            }
        }
    }

    return type;
}

const std::shared_ptr<mega::MegaSyncStall>& StalledIssue::getOriginalStall() const
{
    return originalStall;
}

uint8_t StalledIssue::filesCount() const
{
    return mFiles;
}

uint8_t StalledIssue::foldersCount() const
{
    return mFolders;
}

QSize StalledIssue::getDelegateSize(Type type) const
{
    switch(type)
    {
        case Type::Header:
            return mHeaderDelegateSize;
        case Type::Body:
            return mBodyDelegateSize;
    }
    return QSize(0, 0);
}

void StalledIssue::setDelegateSize(const QSize& newDelegateSize, Type type)
{
    switch(type)
    {
        case Type::Header:
            mHeaderDelegateSize = newDelegateSize;
            break;
        case Type::Body:
            mBodyDelegateSize = newDelegateSize;
            break;
    }
}

void StalledIssue::removeDelegateSize(Type type)
{
    switch(type)
    {
        case Type::Header:
            mHeaderDelegateSize = QSize();
            break;
        case Type::Body:
            mBodyDelegateSize = QSize();
            break;
    }
}

void StalledIssue::resetDelegateSize()
{
    removeDelegateSize(Type::Header);
    removeDelegateSize(Type::Body);
}

bool StalledIssue::isSolved() const
{
    return mIsSolved >= SolveType::POTENTIALLY_SOLVED;
}

bool StalledIssue::isPotentiallySolved() const
{
    return mIsSolved == SolveType::POTENTIALLY_SOLVED;
}

bool StalledIssue::isBeingSolved() const
{
    return mIsSolved == SolveType::BEING_SOLVED;
}

bool StalledIssue::isFailed() const
{
    return mIsSolved == SolveType::FAILED;
}

void StalledIssue::setIsSolved(SolveType type)
{
    if(mIsSolved != type)
    {
        mIsSolved = type;
        // Prevent this one showing again (if they Refresh) until sync has made a full fresh pass
        MegaSyncApp->getMegaApi()->clearStalledPath(originalStall.get());

        resetUIUpdated();
        resetDelegateSize();
    }
}

bool StalledIssue::isAutoSolvable() const
{
    return false;
}

bool StalledIssue::isBeingSolvedByUpload(std::shared_ptr<UploadTransferInfo> info) const
{
    auto result(false);

    auto node = consultCloudData()->getNode();
    if(node)
    {
        info->filename = consultLocalData()->getFileName();
        info->localPath = consultLocalData()->getNativeFilePath();
        info->parentHandle = node->getParentHandle();
        auto transfer = MegaSyncApp->getTransfersModel()->activeUploadTransferFound(info.get());

        result =  transfer != nullptr;
    }

    return result;
}

bool StalledIssue::isBeingSolvedByDownload(std::shared_ptr<DownloadTransferInfo> info) const
{
    auto result(false);

    auto node = consultCloudData()->getNode();
    if(node)
    {
        info->nodeHandle = consultCloudData()->getPathHandle();
        auto transfer = MegaSyncApp->getTransfersModel()->activeDownloadTransferFound(info.get());

        result =  transfer != nullptr;
    }

    return result;
}

void StalledIssue::performFinishAsyncIssueSolving(bool hasFailed)
{
    hasFailed ? setIsSolved(StalledIssue::SolveType::FAILED) : setIsSolved(StalledIssue::SolveType::SOLVED);
    emit asyncIssueSolvingFinished(this);
}

bool StalledIssue::wasAutoResolutionApplied() const
{
    return mAutoResolutionApplied;
}

void StalledIssue::setAutoResolutionApplied(bool newAutoResolutionApplied)
{
    mAutoResolutionApplied = newAutoResolutionApplied;
}

bool StalledIssue::isExpandable() const
{
    return !missingFingerprint();
}

void StalledIssue::startAsyncIssueSolving()
{
    setIsSolved(StalledIssue::SolveType::BEING_SOLVED);
    emit asyncIssueSolvingStarted();
}

bool StalledIssue::missingFingerprint() const
{
    return getReason() == mega::MegaSyncStall::DownloadIssue &&
           consultCloudData() &&
           consultCloudData()->getPath().pathProblem == mega::MegaSyncStall::SyncPathProblem::CloudNodeInvalidFingerprint;
}

bool StalledIssue::canBeIgnored() const
{
    return !mIgnoredPaths.isEmpty();
}

QStringList StalledIssue::getIgnoredFiles() const
{
    return mIgnoredPaths;
}

bool StalledIssue::isFile() const
{
    return mFiles > 0 && mFolders == 0;
}

const LocalStalledIssueDataPtr StalledIssue::consultLocalData() const
{
    return mLocalData;
}

const CloudStalledIssueDataPtr StalledIssue::consultCloudData() const
{
    return mCloudData;
}

const QExplicitlySharedDataPointer<LocalStalledIssueData>& StalledIssue::getLocalData()
{
    return mLocalData;
}

const QExplicitlySharedDataPointer<CloudStalledIssueData>& StalledIssue::getCloudData()
{
    return mCloudData;
}

bool StalledIssue::checkForExternalChanges()
{
    if(!isSolved())
    {
        if(mLocalData)
        {
            QFileInfo fileInfo(mLocalData->getPath().path);
            //Issues without fingerprint may contain
            if(!fileInfo.exists() && !missingFingerprint())
            {
                setIsSolved(StalledIssue::SolveType::POTENTIALLY_SOLVED);
            }
        }

        if(mCloudData)
        {
            auto currentNode = mCloudData->getNode();
            if(currentNode)
            {
                auto node = mCloudData->getNode(true);
                if(!node ||
                   MegaSyncApp->getMegaApi()->isInRubbish(node.get()) ||
                   currentNode->getParentHandle() != node->getParentHandle() ||
                   (missingFingerprint() && (node->getFingerprint() != nullptr)))
                {
                    setIsSolved(StalledIssue::SolveType::POTENTIALLY_SOLVED);
                }
            }
        }
    }

    return isPotentiallySolved();
}

QStringList StalledIssue::getLocalFiles()
{
    QStringList files;
    if(getLocalData())
    {
        auto file = getLocalData()->getFilePath();
        if(!file.isEmpty())
        {
            files << file;
        }
    }

    return files;
}

void StalledIssue::setIsFile(const QString& path, bool isLocal)
{
    if(isLocal)
    {
        QFileInfo fileInfo(path);
        fileInfo.isFile() ? mFiles++ : mFolders++;
    }
    else
    {
        std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByPath(path.toStdString().c_str()));
        if(node)
        {
            node->isFile()  ? mFiles++ : mFolders++;
        }
    }
}

bool StalledIssue::needsUIUpdate(Type type) const
{
    switch(type)
    {
        case Type::Header:
            return mNeedsUIUpdate.first;
        case Type::Body:
            return mNeedsUIUpdate.second;
    }

    return true;
}

void StalledIssue::UIUpdated(Type type)
{
    switch(type)
    {
        case Type::Header:
            mNeedsUIUpdate.first = false;
            break;
        case Type::Body:
        {
            if(mNeedsUIUpdate.second == false)
            {
                return;
            }
            mNeedsUIUpdate.second = false;
            break;
        }
    }
}

void StalledIssue::resetUIUpdated()
{
    if(mNeedsUIUpdate.first && mNeedsUIUpdate.second)
    {
        return;
    }

    mNeedsUIUpdate = qMakePair(true, true);
    emit dataUpdated(this);
}

//By default, stalled issues don't show file attributes (size, time modified)...´
bool StalledIssue::UIShowFileAttributes() const
{
    return false;
}

void StalledIssue::createFileWatcher()
{
    if(UIShowFileAttributes())
    {
        mFileSystemWatcher->createFileWatcher();
    }
}

void StalledIssue::removeFileWatcher()
{
    if(UIShowFileAttributes())
    {
        mFileSystemWatcher->removeFileWatcher();
    }
}

mega::MegaHandle StalledIssue::firstSyncId() const
{
    if(mSyncIds.isEmpty())
    {
        return mega::INVALID_HANDLE;
    }

    return (*mSyncIds.begin());
}

mega::MegaSyncStall::SyncStallReason StalledIssue::getReason() const
{
    return mReason;
}

QString StalledIssue::getFileName(bool preferCloud) const
{
    QString fileName;

    if (preferCloud)
    {
        if (mCloudData)
        {
            fileName = mCloudData->getFileName();
        }
    }
    else
    {
        if (mLocalData)
        {
            fileName = mLocalData->getFileName();
        }
    }

    if(fileName.isEmpty())
    {
        if(mLocalData)
        {
            fileName = mLocalData->getFileName();
        }
        else if(mCloudData)
        {
            fileName = mCloudData->getFileName();
        }
    }

    return fileName;
}

bool StalledIssue::operator==(const StalledIssue& data)
{
    bool equal(true);

    equal &= (mLocalData == data.mLocalData);
    equal &= (mCloudData == data.mCloudData);

    return equal;
}

void StalledIssue::updateIssue(const mega::MegaSyncStall* stallIssue)
{
    mLocalData.reset();
    mCloudData.reset();

    mIsSolved = SolveType::UNSOLVED;

    fillIssue(stallIssue);
    endFillingIssue();
}

bool StalledIssue::isUnsolved() const
{
    return mIsSolved == SolveType::UNSOLVED;
}

StalledIssueFilterCriterion StalledIssue::getCriterionByReason(mega::MegaSyncStall::SyncStallReason reason)
{
    switch (reason)
    {
        case mega::MegaSyncStall::SyncStallReason::FileIssue:
        {
            return StalledIssueFilterCriterion::ITEM_TYPE_CONFLICTS;
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::NamesWouldClashWhenSynced:
        {
            return StalledIssueFilterCriterion::NAME_CONFLICTS;
            break;
        }
        case mega::MegaSyncStall::SyncStallReason::MoveOrRenameCannotOccur:
        case mega::MegaSyncStall::SyncStallReason::DeleteOrMoveWaitingOnScanning:
        case mega::MegaSyncStall::SyncStallReason::DeleteWaitingOnMoves:
        case mega::MegaSyncStall::SyncStallReason::UploadIssue:
        case mega::MegaSyncStall::SyncStallReason::DownloadIssue:
        case mega::MegaSyncStall::SyncStallReason::CannotCreateFolder:
        case mega::MegaSyncStall::SyncStallReason::CannotPerformDeletion:
        case mega::MegaSyncStall::SyncStallReason::SyncItemExceedsSupportedTreeDepth:
        case mega::MegaSyncStall::SyncStallReason::FolderMatchedAgainstFile:
        case mega::MegaSyncStall::SyncStallReason::LocalAndRemoteChangedSinceLastSyncedState_userMustChoose:
        case mega::MegaSyncStall::SyncStallReason::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose:
        default:
        {
            return StalledIssueFilterCriterion::OTHER_CONFLICTS;
            break;
        }
    }
}
