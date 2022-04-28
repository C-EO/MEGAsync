#include "StalledIssuesModel.h"

#include "MegaApplication.h"

StalledIssuesReceiver::StalledIssuesReceiver(QObject *parent) : QObject(parent), mega::MegaRequestListener()
{
}

void StalledIssuesReceiver::processStalledIssues()
{
    StalledIssuesList auxList;

    if(mCacheStalledIssues.size() > 2000)
    {
        for(auto index = 0; index < 2000
            && !mCacheStalledIssues.isEmpty(); ++index)
        {
            auto& firstItem = mCacheStalledIssues.first();            
            auxList.append(firstItem);
            mCacheStalledIssues.removeOne(firstItem);

        }
    }
    else
    {
        auxList = mCacheStalledIssues;
        mCacheStalledIssues.clear();

    }

    emit stalledIssuesReady(auxList);
}

void StalledIssuesReceiver::onRequestFinish(mega::MegaApi*, mega::MegaRequest *request, mega::MegaError*)
{
    if (auto ptr = request->getMegaSyncProblems())
    {
        QMutexLocker lock(&mCacheMutex);

        if (mega::MegaSyncNameConflictList* cl = ptr->nameConflicts())
        {
            for (int i = 0; i < cl->size(); ++i)
            {
                auto nameConflictStall = cl->get(i);

                ConflictedNamesStalledIssue conflictNameItem(nameConflictStall);
                mCacheStalledIssues.append(conflictNameItem);
            }
        }

        if (mega::MegaSyncStallList* sl = ptr->stalls())
        {
            QList<const mega::MegaSyncStall*> deleteWaitingOnMovesStalls;

            for (int i = 0; i < sl->size(); ++i)
            {
                auto stall = sl->get(i);
                bool createStalledIssue(true);

                if(stall->reason() == mega::MegaSyncStall::SyncStallReason::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose
                        || stall->reason() == mega::MegaSyncStall::SyncStallReason::LocalAndRemoteChangedSinceLastSyncedState_userMustChoose)
                {
                    for(int index = 0; index < mCacheStalledIssues.size(); ++index)
                    {
                        auto& issue = mCacheStalledIssues[index];

                        if(issue.getReason() == stall->reason())
                        {
                           if(stall->isCloud())
                            {
                                if(QString::fromStdString(stall->indexPath()) == issue.getStalledIssueData()->mCloudPath
                                        && QString::fromStdString(stall->localPath()) == issue.getStalledIssueData()->mIndexPath.path)
                                {
                                    issue.addStalledIssueData(StalledIssueDataPtr(new StalledIssueData(stall)));
                                    createStalledIssue = false;
                                    break;
                                }
                            }
                            else
                            {
                                if(QString::fromStdString(stall->indexPath()) == issue.getStalledIssueData()->mLocalPath
                                        && QString::fromStdString(stall->cloudPath()) == issue.getStalledIssueData()->mIndexPath.path)
                                {
                                    issue.addStalledIssueData(StalledIssueDataPtr(new StalledIssueData(stall)));
                                    createStalledIssue = false;
                                    break;
                                }
                            }
                        }
                    }
                }
                else if(stall->reason() == mega::MegaSyncStall::SyncStallReason::DeleteWaitingOnMoves || stall->reason() == mega::MegaSyncStall::SyncStallReason::MoveNeedsDestinationNodeProcessing)
                {
                    deleteWaitingOnMovesStalls.append(stall);
                    continue;
                }

                if(createStalledIssue)
                {
                    StalledIssue d (StalledIssueDataPtr(new StalledIssueData(stall)), stall->reason());
                    mCacheStalledIssues.append(d);
                }
            }

            processDeleteWaitingOnMoves(deleteWaitingOnMovesStalls);
        }

        processStalledIssues();
    }
}

void StalledIssuesReceiver::processDeleteWaitingOnMoves(QList<const mega::MegaSyncStall *> stalls)
{
    foreach(auto stall, stalls)
    {
        for(int index = 0; index < mCacheStalledIssues.size(); ++index)
        {
            auto& issue = mCacheStalledIssues[index];

            if(issue.getReason() == mega::MegaSyncStall::SyncStallReason::ApplyMoveNeedsOtherSideParentFolderToExist)
            {
                if(issue.getStalledIssueData()->mIsCloud)
                {
                    auto remoteData = issue.getStalledIssueData();

                    if(stall->isCloud())
                    {
                        if(QString::fromUtf8(stall->cloudPath()) == remoteData->mIndexPath.path)
                        {
                            StalledIssueDataPtr localData(new StalledIssueData(nullptr));
                            issue.addStalledIssueData(localData);

                            QFileInfo localTargetInfo(remoteData->mLocalPath);
                            localData->mMovePath.path = QDir::toNativeSeparators(localTargetInfo.dir().path());
                            localData->mMovePath.isMissing = true;

                            QFileInfo localSourceInfo(QString::fromUtf8(stall->localPath()));
                            localData->mIndexPath.path = QDir::toNativeSeparators(localSourceInfo.dir().path());

                            QFileInfo cloudSourceInfo(remoteData->mCloudPath);
                            remoteData->mIndexPath.path = cloudSourceInfo.dir().path();

                            QFileInfo cloudTargetInfo(QString::fromUtf8(stall->cloudPath()));
                            remoteData->mMovePath.path = cloudTargetInfo.dir().path();

                            break;
                        }
                    }
                }
                else
                {
                    if(stall->isCloud())
                    {
                        auto localPath = QString::fromUtf8(stall->localPath());
                        auto nativeLocalPath = QDir::toNativeSeparators(localPath);

                        if(nativeLocalPath == issue.getStalledIssueData()->mMovePath.path)
                        {
                            auto localData = issue.getStalledIssueData();

                            StalledIssueDataPtr remoteData (new StalledIssueData(nullptr));
                            remoteData->mIsCloud = true;

                            QFileInfo remoteInfo(localData->mCloudPath);
                            remoteData->mIndexPath.path = remoteInfo.dir().path();

                            remoteData->mMovePath.path = QString::fromUtf8(stall->indexPath());
                            remoteData->mMovePath.isMissing = true;

                            issue.addStalledIssueData(remoteData);
                            break;
                        }
                    }
                    else
                    {
                        if(QString::fromStdString(stall->indexPath()) == issue.getStalledIssueData()->mLocalPath)
                        {
                            auto localData = issue.getStalledIssueData();
                            QFileInfo moveInfo(localData->mIndexPath.path);
                            localData->mMovePath.path = QDir::toNativeSeparators(moveInfo.dir().path());

                            QFileInfo localInfo(localData->mLocalPath);
                            localData->mIndexPath.path = QDir::toNativeSeparators(localInfo.dir().path());

                            localData->mCloudPath = QString::fromUtf8(stall->cloudPath());
                            break;
                        }
                    }
                }
            }
            else if(issue.getReason() == mega::MegaSyncStall::SyncStallReason::LocalFolderNotScannable)
            {
                auto localData = issue.getStalledIssueData();

                if(stall->isCloud())
                {
                    if(QString::fromUtf8(stall->cloudPath()) == localData->mIndexPath.path)
                    {
                        StalledIssueDataPtr remoteData(new StalledIssueData(nullptr));
                        issue.addStalledIssueData(remoteData);

                        QFileInfo remoteTargetInfo(QString::fromUtf8(stall->cloudPath()));
                        remoteData->mMovePath.path = remoteTargetInfo.dir().path();

                        QFileInfo remoteSourceInfo(QString::fromUtf8(stall->indexPath()));
                        remoteData->mIndexPath.path = remoteSourceInfo.dir().path();

                        remoteData->mIndexPath.path = QDir::toNativeSeparators(localData->mIndexPath.path);

                        QFileInfo localTargetInfo(QString::fromUtf8(stall->localPath()));
                        localData->mMovePath.path = QDir::toNativeSeparators(localTargetInfo.dir().path());
                        localData->mMovePath.isBlocked = true;

                        break;
                    }
                }
            }
        }
    }
}

StalledIssuesModel::StalledIssuesModel(QObject *parent) : QAbstractItemModel(parent)
   , mMegaApi (MegaSyncApp->getMegaApi())
   , mHasStalledIssues(false)
{
    mStalledIssuesThread = new QThread();
    mStalledIssuedReceiver = new StalledIssuesReceiver();

    mRequestListener = new mega::QTMegaRequestListener(mMegaApi, mStalledIssuedReceiver);
    mStalledIssuedReceiver->moveToThread(mStalledIssuesThread);
    mRequestListener->moveToThread(mStalledIssuesThread);
    mMegaApi->addRequestListener(mRequestListener);

    mGlobalListener = new mega::QTMegaGlobalListener(mMegaApi,this);
    mMegaApi->addGlobalListener(mGlobalListener);

    mStalledIssuesThread->start();

    connect(mStalledIssuedReceiver, &StalledIssuesReceiver::stalledIssuesReady,
            this, &StalledIssuesModel::onProcessStalledIssues,
            Qt::QueuedConnection);
}

StalledIssuesModel::~StalledIssuesModel()
{
    mMegaApi->removeRequestListener(mRequestListener);
    mMegaApi->removeGlobalListener(mGlobalListener);

    mStalledIssuesThread->quit();
    mStalledIssuesThread->deleteLater();
    mStalledIssuedReceiver->deleteLater();

    mRequestListener->deleteLater();
}

void StalledIssuesModel::onProcessStalledIssues(StalledIssuesList stalledIssues)
{
    if(!stalledIssues.isEmpty())
    {
        auto totalRows = rowCount(QModelIndex());
        auto rowsToBeInserted(static_cast<int>(stalledIssues.size()));

        beginInsertRows(QModelIndex(), totalRows, totalRows + rowsToBeInserted - 1);

        for (auto it = stalledIssues.begin(); it != stalledIssues.end();)
        {
            mStalledIssues.append((*it));
            mStalledIssuesByOrder.insert(&(*it), rowCount(QModelIndex()) - 1);

            stalledIssues.removeOne((*it));
            it++;
        }

        endInsertRows();
        emit stalledIssuesCountChanged();
    }

    emit stalledIssuesReceived(true);
}

void StalledIssuesModel::updateStalledIssues()
{
    //For a future, add a Loading Windows here
    reset();

    if (mMegaApi->isSyncStalled())
    {
        mMegaApi->getSyncProblems(nullptr, true);
    }
}

void StalledIssuesModel::onGlobalSyncStateChanged(mega::MegaApi* api)
{
    mHasStalledIssues = api->isSyncStalled();
    emit stalledIssuesReceived(api->isSyncStalled());
}

Qt::DropActions StalledIssuesModel::supportedDropActions() const
{
    return Qt::IgnoreAction;
}

bool StalledIssuesModel::hasChildren(const QModelIndex &parent) const
{
    auto stalledIssueItem = static_cast<StalledIssue*>(parent.internalPointer());
    if (stalledIssueItem)
    {
        return false;
    }


    return true;
}

int StalledIssuesModel::rowCount(const QModelIndex &parent) const
{
   if(!parent.isValid())
   {
       return mStalledIssues.size();
   }
   else
   {
       return 1;
   }

   return 0;
}

int StalledIssuesModel::columnCount(const QModelIndex &parent) const
{
   return 1;
}

QVariant StalledIssuesModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        auto stalledIssueItem = static_cast<StalledIssue*>(index.internalPointer());
        if (stalledIssueItem)
        {
            return QVariant::fromValue((*stalledIssueItem));
        }
        else
        {
            return QVariant::fromValue(StalledIssue(mStalledIssues.at(index.row())));
        }
    }

    return QVariant();
}

QModelIndex StalledIssuesModel::parent(const QModelIndex &index) const
{
    if(!index.isValid())
    {
        return QModelIndex();
    }

    auto stalledIssueItem = static_cast<StalledIssue*>(index.internalPointer());
    if (!stalledIssueItem)
    {
        return QModelIndex();
    }

    auto row = mStalledIssuesByOrder.value(&(*stalledIssueItem),-1);
    if(row >= 0)
    {
        return createIndex(row, 0);
    }

    return QModelIndex();
}

QModelIndex StalledIssuesModel::index(int row, int column, const QModelIndex &parent) const
{
    if(parent.isValid())
    {
        auto& stalledIssue = mStalledIssues[parent.row()];
        return createIndex(0, 0, &stalledIssue);
    }
    else
    {
        return (row < rowCount(QModelIndex())) ?  createIndex(row, column) : QModelIndex();
    }
}

Qt::ItemFlags StalledIssuesModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

void StalledIssuesModel::finishStalledIssues(const QModelIndexList &indexes)
{
    auto indexesToFinish(indexes);
    removeRows(indexesToFinish);
}

void StalledIssuesModel::removeRows(QModelIndexList &indexesToRemove)
{
    std::sort(indexesToRemove.begin(), indexesToRemove.end(),[](QModelIndex check1, QModelIndex check2){
        return check1.row() > check2.row();
    });

    // First clear finished transfers (remove rows), then cancel the others.
    // This way, there is no risk of messing up the rows order with cancel requests.
    int count (0);
    int row (indexesToRemove.last().row());
    for (auto index : indexesToRemove)
    {
        // Init row with row of first tag
        if (count == 0)
        {
            row = index.row();
        }

        // If rows are non-contiguous, flush and start from item
        if (row != index.row())
        {
            removeRows(row + 1, count, QModelIndex());
            count = 0;
            row = index.row();
        }

        // We have at least one row
        count++;
        row--;
    }
    // Flush pooled rows (start at row + 1).
    // This happens when the last item processed is in a finished state.
    if (count > 0)
    {
        removeRows(row + 1, count, QModelIndex());
    }

    updateStalledIssuedByOrder();
}

bool StalledIssuesModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent == QModelIndex() && count > 0 && row >= 0)
    {
        beginRemoveRows(parent, row, row + count - 1);

        for (auto i (0); i < count; ++i)
        {
            mStalledIssues.removeAt(i);
        }

        endRemoveRows();

        emit stalledIssuesCountChanged();

        return true;
    }
    else
    {
        return false;
    }
}

void StalledIssuesModel::updateStalledIssuedByOrder()
{
    mStalledIssuesByOrder.clear();

    //Recalculate rest of items
    for(int row = 0; row < rowCount(QModelIndex()); ++row)
    {
        auto item = mStalledIssues.at(row);
        mStalledIssuesByOrder.insert(&item, row);
    }
}

bool StalledIssuesModel::hasStalledIssues() const
{
    return mHasStalledIssues;
}

void StalledIssuesModel::reset()
{
    beginResetModel();

    mStalledIssues.clear();
    mStalledIssuesByOrder.clear();

    endResetModel();

    emit stalledIssuesCountChanged();
}
