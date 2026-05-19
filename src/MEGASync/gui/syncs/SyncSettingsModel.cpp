#include "SyncSettingsModel.h"

#include "SyncInfo.h"

#include <QCoreApplication>

SyncSettingsModel::SyncSettingsModel(QObject* parent):
    QAbstractListModel(parent),
    mSyncInfo(SyncInfo::instance())
{
    connect(mSyncInfo, &SyncInfo::syncStateChanged, this, &SyncSettingsModel::insertSync);
    connect(mSyncInfo, &SyncInfo::syncStatsUpdated, this, &SyncSettingsModel::updateSyncStats);
    connect(mSyncInfo, &SyncInfo::syncRemoved, this, &SyncSettingsModel::removeSync);

    mList = mSyncInfo->getSyncSettingsByType(mega::MegaSync::SyncType::TYPE_TWOWAY);
}

void SyncSettingsModel::insertSync(std::shared_ptr<SyncSettings> sync)
{
    if (sync->getType() != mega::MegaSync::SyncType::TYPE_TWOWAY)
    {
        return;
    }

    auto foundSyncIt = std::find_if(mList.cbegin(),
                                    mList.cend(),
                                    [&sync](auto& listSync)
                                    {
                                        return (sync == listSync);
                                    });

    if (foundSyncIt != mList.cend())
    {
        auto row = std::distance(mList.cbegin(), foundSyncIt);
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

void SyncSettingsModel::sendDataChanged(int row)
{
    const auto modelIndex = index(row, 0, QModelIndex());

    emit dataChanged(modelIndex,
                     modelIndex,
                     QVector<int>() << Role::FolderRole << Role::StatusRole << Role::StatusId
                                    << Role::ErrorMessage << Role::NameRole << Role::ErrorId);
}

void SyncSettingsModel::updateSyncStats(std::shared_ptr<::mega::MegaSyncStats> stats)
{
    auto foundSyncIt = std::find_if(mList.cbegin(),
                                    mList.cend(),
                                    [&stats](auto& listSync)
                                    {
                                        return (stats->getBackupId() == listSync->backupId());
                                    });

    if (foundSyncIt != mList.cend())
    {
        auto row = std::distance(mList.cbegin(), foundSyncIt);
        sendDataChanged(static_cast<int>(row));
    }
}

void SyncSettingsModel::removeSync(std::shared_ptr<SyncSettings> sync)
{
    auto foundSyncIt = std::find_if(mList.cbegin(),
                                    mList.cend(),
                                    [&sync](auto& listSync)
                                    {
                                        return (sync == listSync);
                                    });

    if (foundSyncIt != mList.cend())
    {
        auto row = std::distance(mList.cbegin(), foundSyncIt);
        auto pos = static_cast<int>(row);

        beginRemoveRows(QModelIndex(), pos, pos);
        mList.removeOne((*foundSyncIt));
        endRemoveRows();
    }
}

QHash<int, QByteArray> SyncSettingsModel::roleNames() const
{
    return {
        {NameRole, "name"},
        {FolderRole, "folder"},
        {StatusRole, "status"},
        {ErrorMessage, "error"},
        {ErrorId, "error_id"},
    };
}

int SyncSettingsModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return static_cast<int>(mList.size());
}

QVariant SyncSettingsModel::data(const QModelIndex& index, int role) const
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

QString SyncSettingsModel::getErrorMessage(std::shared_ptr<SyncSettings> sync) const
{
    std::unique_ptr<const char[]> syncErrorText(
        mega::MegaSync::getMegaSyncErrorCode(sync->getError()));

    return QCoreApplication::translate("MegaSyncError", syncErrorText.get());
}

std::shared_ptr<SyncSettings> SyncSettingsModel::getSync(int index) const
{
    return mList.at(index);
}

SyncSettingsModel::SyncStates SyncSettingsModel::getState(std::shared_ptr<SyncSettings> sync) const
{
    switch (sync->getRunState())
    {
        case ::mega::MegaSync::RUNSTATE_PENDING:
        {
            return SyncStates::PENDING;
        }
        case ::mega::MegaSync::RUNSTATE_LOADING:
        {
            return SyncStates::LOADING;
        }
        case ::mega::MegaSync::RUNSTATE_SUSPENDED:
        {
            if (sync->getError())
            {
                return SyncStates::FAIL;
            }
            else
            {
                return SyncStates::SUSPENDED;
            }
        }
        case ::mega::MegaSync::RUNSTATE_DISABLED:
        {
            return SyncStates::FAIL;
        }
        case ::mega::MegaSync::RUNSTATE_RUNNING:
        {
            auto it = mSyncInfo->mSyncStatsMap.find(sync->backupId());
            if (it != mSyncInfo->mSyncStatsMap.end())
            {
                auto stats = it->second;
                if (stats->isScanning())
                {
                    return SyncStates::SCANNING;
                }
                else if (stats->isSyncing())
                {
                    return SyncStates::SYNCING;
                }
            }
            break;
        }
    }

    return SyncStates::SYNCED;
}
