#include "BackupCandidates.h"

#include "BackupsController.h"
#include "Utilities.h"

const char* BackupCandidates::Data::SIZE_READY = "size_ready";

BackupCandidates::Data::Data(const QString& folder, const QString& displayName, bool selected):
    mFolder(folder),
    mName(displayName),
    mSelected(selected),
    mDone(false),
    mError(0),
    mFolderSize(FileFolderAttributes::NOT_READY),
    mSdkError(-1),
    mSyncError(-1)
{}

QHash<int, QByteArray> BackupCandidates::Data::roleNames()
{
    static QHash<int, QByteArray> roles{
        {BackupCandidates::NAME_ROLE,       "name"     },
        {BackupCandidates::FOLDER_ROLE,     "folder"   },
        {BackupCandidates::SIZE_ROLE,       "size"     },
        {BackupCandidates::SIZE_READY_ROLE, "sizeReady"},
        {BackupCandidates::SELECTED_ROLE,   "selected" },
        {BackupCandidates::DONE_ROLE,       "done"     },
        {BackupCandidates::ERROR_ROLE,      "error"    }
    };

    return roles;
}

//////////////////////////////////////////////////////////////////
QString BackupCandidates::getTotalSize() const
{
    return Utilities::getSizeStringLocalized(mBackupsTotalSize);
}

bool BackupCandidates::getIsTotalSizeReady() const
{
    return mTotalSizeReady;
}

void BackupCandidates::setIsTotalSizeReady(bool totalSizeReady)
{
    if (mTotalSizeReady != totalSizeReady)
    {
        mTotalSizeReady = totalSizeReady;
        emit totalSizeReadyChanged();
    }
}

Qt::CheckState BackupCandidates::getCheckAllState() const
{
    return mCheckAllState;
}

void BackupCandidates::setCheckAllState(Qt::CheckState state)
{
    if (mCheckAllState != state)
    {
        mCheckAllState = state;
        emit checkAllStateChanged();
    }
}

void BackupCandidates::setGlobalError(BackupErrorCode error)
{
    if (mGlobalError != error)
    {
        mGlobalError = error;
        emit globalErrorChanged();
    }
}

int BackupCandidates::getGlobalError() const
{
    return mGlobalError;
}

std::shared_ptr<BackupCandidates::Data> BackupCandidates::getBackupCandidate(int index) const
{
    return mBackupCandidatesList.at(index);
}

std::shared_ptr<BackupCandidates::Data>
    BackupCandidates::getBackupCandidateByFolder(const QString& folder) const
{
    foreach(auto& candidate, mBackupCandidatesList)
    {
        if (candidate->mFolder == folder)
        {
            return candidate;
        }
    }

    return nullptr;
}

QList<std::shared_ptr<BackupCandidates::Data>> BackupCandidates::getBackupCandidates() const
{
    return mBackupCandidatesList;
}

int BackupCandidates::size() const
{
    return mBackupCandidatesList.size();
}

int BackupCandidates::getRow(std::shared_ptr<BackupCandidates::Data> candidate) const
{
    return mBackupCandidatesList.indexOf(candidate);
}

int BackupCandidates::getRow(const QString& folder) const
{
    foreach(auto& candidate, mBackupCandidatesList)
    {
        if (candidate->mFolder == folder)
        {
            return getRow(candidate);
        }
    }

    return -1;
}

void BackupCandidates::addBackupCandidate(std::shared_ptr<BackupCandidates::Data> backupCandidate)
{
    mBackupCandidatesList.append(backupCandidate);
}

void BackupCandidates::removeBackupCandidate(int index)
{
    mBackupCandidatesList.removeAt(index);
}

bool BackupCandidates::removeBackupCandidate(const QString& folder)
{
    auto row(getRow(folder));
    if (row >= 0)
    {
        mBackupCandidatesList.removeAt(row);
        return true;
    }

    return false;
}

int BackupCandidates::SDKConflictCount() const
{
    return mSDKConflictCount;
}

void BackupCandidates::setSDKConflictCount(int newSdkConflictCount)
{
    mSDKConflictCount = newSdkConflictCount;
}

int BackupCandidates::remoteConflictCount() const
{
    return mRemoteConflictCount;
}

void BackupCandidates::setRemoteConflictCount(int newRemoteConflictCount)
{
    mRemoteConflictCount = newRemoteConflictCount;
}

int BackupCandidates::selectedRowsTotal() const
{
    return mSelectedRowsTotal;
}

void BackupCandidates::setSelectedRowsTotal(int newSelectedRowsTotal)
{
    mSelectedRowsTotal = newSelectedRowsTotal;

    if (mSelectedRowsTotal == 0)
    {
        emit noneSelected();
    }
}

long long BackupCandidates::backupsTotalSize() const
{
    return mBackupsTotalSize;
}

void BackupCandidates::setBackupsTotalSize(long long newBackupsTotalSize)
{
    if (mBackupsTotalSize != newBackupsTotalSize)
    {
        mBackupsTotalSize = newBackupsTotalSize;
        emit totalSizeChanged();
    }
}

const QString& BackupCandidates::getConflictsNotificationText() const
{
    return mConflictsNotificationText;
}

void BackupCandidates::setConflictsNotificationText(const QString& text)
{
    if (mConflictsNotificationText != text)
    {
        mConflictsNotificationText = text;
    }
}
