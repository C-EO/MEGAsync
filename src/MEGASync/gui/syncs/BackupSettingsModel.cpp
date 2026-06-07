#include "BackupSettingsModel.h"

#include "SyncInfo.h"

#include <QCoreApplication>

BackupSettingsModel::BackupSettingsModel(QObject* parent):
    QAbstractListModel(parent),
    mSyncInfo(SyncInfo::instance())
{
    mQCollator.setNumericMode(true);
    mQCollator.setCaseSensitivity(Qt::CaseInsensitive);

    connect(mSyncInfo, &SyncInfo::syncStateChanged, this, &BackupSettingsModel::insertBackup);
    connect(mSyncInfo, &SyncInfo::syncStatsUpdated, this, &BackupSettingsModel::updateBackupStats);
    connect(mSyncInfo, &SyncInfo::syncRemoved, this, &BackupSettingsModel::removeBackup);

    mList = mSyncInfo->getSyncSettingsByType(mega::MegaSync::SyncType::TYPE_BACKUP);
    sortByName(true);
}

void BackupSettingsModel::sortByName(bool ascending)
{
    mSortAscending = ascending;

    beginResetModel();
    std::sort(mList.begin(),
              mList.end(),
              [this, ascending](const auto& sync1, const auto& sync2)
              {
                  const auto result =
                      mQCollator.compare(sync1->name(false, true), sync2->name(false, true));
                  return ascending ? result <= 0 : result > 0;
              });
    endResetModel();
}

void BackupSettingsModel::sortByStatus(bool ascending)
{
    mSortAscending = ascending;

    beginResetModel();
    std::sort(mList.begin(),
              mList.end(),
              [this, ascending](const auto& sync1, const auto& sync2)
              {
                  return ascending ? getState(sync1) <= getState(sync2) :
                                     getState(sync1) > getState(sync2);
              });
    endResetModel();
}

void BackupSettingsModel::insertBackup(std::shared_ptr<SyncSettings> sync)
{
    if (sync->getType() != mega::MegaSync::SyncType::TYPE_BACKUP)
    {
        return;
    }

    auto foundIt = std::find_if(mList.cbegin(),
                                mList.cend(),
                                [&sync](auto& listSync)
                                {
                                    return (sync == listSync);
                                });

    if (foundIt != mList.cend())
    {
        auto row = std::distance(mList.cbegin(), foundIt);
        sendDataChanged(static_cast<int>(row));
    }
    else
    {
        beginInsertRows(QModelIndex(),
                        static_cast<int>(mList.size()),
                        static_cast<int>(mList.size()));
        mList.append(sync);
        endInsertRows();
    }
}

void BackupSettingsModel::sendDataChanged(int row)
{
    const auto modelIndex = index(row, 0, QModelIndex());

    emit dataChanged(modelIndex,
                     modelIndex,
                     QVector<int>() << Role::FolderRole << Role::StatusRole << Role::StatusId
                                    << Role::ErrorMessage << Role::NameRole << Role::ErrorId);
}

void BackupSettingsModel::updateBackupStats(std::shared_ptr<::mega::MegaSyncStats> stats)
{
    auto foundIt = std::find_if(mList.cbegin(),
                                mList.cend(),
                                [&stats](auto& listSync)
                                {
                                    return (stats->getBackupId() == listSync->backupId());
                                });

    if (foundIt != mList.cend())
    {
        auto row = std::distance(mList.cbegin(), foundIt);
        sendDataChanged(static_cast<int>(row));
    }
}

void BackupSettingsModel::removeBackup(std::shared_ptr<SyncSettings> sync)
{
    auto foundIt = std::find_if(mList.cbegin(),
                                mList.cend(),
                                [&sync](auto& listSync)
                                {
                                    return (sync == listSync);
                                });

    if (foundIt != mList.cend())
    {
        auto row = std::distance(mList.cbegin(), foundIt);
        auto pos = static_cast<int>(row);

        beginRemoveRows(QModelIndex(), pos, pos);
        mList.removeOne((*foundIt));
        endRemoveRows();
    }
}

QHash<int, QByteArray> BackupSettingsModel::roleNames() const
{
    return {
        {NameRole, "name"},
        {FolderRole, "folder"},
        {StatusRole, "status"},
        {ErrorMessage, "error"},
        {ErrorId, "error_id"},
    };
}

int BackupSettingsModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return static_cast<int>(mList.size());
}

QVariant BackupSettingsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    const auto row = index.row();

    if (row < 0 || row >= rowCount())
    {
        return {};
    }

    const auto& sync = mList[row];

    switch (role)
    {
        case FolderRole:
            return sync->getLocalFolder();

        case NameRole:
            return sync->name(false, true);

        case StatusRole:
            return getState(sync);

        case ErrorId:
            return sync->getError();

        case ErrorMessage:
            return getErrorMessage(sync);

        default:
            return {};
    }
}

QString BackupSettingsModel::getErrorMessage(std::shared_ptr<SyncSettings> sync) const
{
    std::unique_ptr<const char[]> syncErrorText(
        mega::MegaSync::getMegaSyncErrorCode(sync->getError()));

    return QCoreApplication::translate("MegaSyncError", syncErrorText.get());
}

std::shared_ptr<SyncSettings> BackupSettingsModel::getBackup(int index) const
{
    return mList.at(index);
}

BackupSettingsModel::BackupStates
    BackupSettingsModel::getState(std::shared_ptr<SyncSettings> sync) const
{
    switch (sync->getRunState())
    {
        case ::mega::MegaSync::RUNSTATE_PENDING:
        {
            return BackupStates::PENDING;
        }
        case ::mega::MegaSync::RUNSTATE_LOADING:
        {
            return BackupStates::LOADING;
        }
        case ::mega::MegaSync::RUNSTATE_SUSPENDED:
        {
            if (sync->getError())
            {
                return BackupStates::FAIL;
            }
            else
            {
                return BackupStates::SUSPENDED;
            }
        }
        case ::mega::MegaSync::RUNSTATE_DISABLED:
        {
            return BackupStates::FAIL;
        }
        case ::mega::MegaSync::RUNSTATE_RUNNING:
        {
            auto it = mSyncInfo->mSyncStatsMap.find(sync->backupId());
            if (it != mSyncInfo->mSyncStatsMap.end())
            {
                auto stats = it->second;
                if (stats->isScanning())
                {
                    return BackupStates::SCANNING;
                }
                else if (stats->isSyncing())
                {
                    return BackupStates::BACKING_UP;
                }
            }
            break;
        }
    }

    return BackupStates::BACKED_UP;
}
