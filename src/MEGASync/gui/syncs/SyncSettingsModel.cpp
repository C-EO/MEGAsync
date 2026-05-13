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
                                    << Role::ErrorMessage << Role::NameRole);
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
    return {{NameRole, "name"},
            {FolderRole, "folder"},
            {StatusRole, "status"},
            {StatusId, "statusid"},
            {ErrorMessage, "error"}};
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

        case StatusId:
            return getStatusId(sync);

        case ErrorMessage:
            return getErrorMessage(sync);

        default:
            return {};
    }
}

QString SyncSettingsModel::getErrorMessage(std::shared_ptr<SyncSettings> sync) const
{
    QString errorMessage;
    if (SyncStates::ERROR == getStatusId(sync))
    {
        std::unique_ptr<const char[]> syncErrorText(
            mega::MegaSync::getMegaSyncErrorCode(sync->getError()));
        QString toolTip;
        errorMessage += QCoreApplication::translate("MegaSyncError", syncErrorText.get());
    }

    return errorMessage;
}

SyncSettingsModel::SyncStates
    SyncSettingsModel::getStatusId(std::shared_ptr<SyncSettings> sync) const
{
    auto statusId = sync->getRunState();
    if (statusId == ::mega::MegaSync::RUNSTATE_SUSPENDED && sync->getError())
    {
        statusId = SyncStates::ERROR;
    }

    return static_cast<SyncStates>(statusId);
}

std::shared_ptr<SyncSettings> SyncSettingsModel::getSync(int index) const
{
    return mList.at(index);
}

QString SyncSettingsModel::getState(std::shared_ptr<SyncSettings> sync) const
{
    QString s;
    switch (sync->getRunState())
    {
        case ::mega::MegaSync::RUNSTATE_PENDING:
            [[fallthrough]];
        case ::mega::MegaSync::RUNSTATE_LOADING:
        {
            s = tr("Loading");
            break;
        }
        case ::mega::MegaSync::RUNSTATE_SUSPENDED:
        {
            if (sync->getError())
            {
                s = tr("Disabled");
            }
            else
            {
                s = tr("Paused");
            }
            break;
        }
        case ::mega::MegaSync::RUNSTATE_DISABLED:
        {
            s = tr("Stopped");
            break;
        }
        case ::mega::MegaSync::RUNSTATE_RUNNING:
        {
            auto it = mSyncInfo->mSyncStatsMap.find(sync->backupId());
            if (it != mSyncInfo->mSyncStatsMap.end())
            {
                ::mega::MegaSyncStats& stats = *it->second;
                if (stats.isScanning())
                {
                    s = tr("Scanning");
                }
                else if (stats.isSyncing())
                {
                    s = tr("Syncing");
                }
                else
                {
                    s = tr("Synced");
                }
            }
            break;
        }
    }
    return s;
}
