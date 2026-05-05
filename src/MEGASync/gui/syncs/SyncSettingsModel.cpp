#include "SyncSettingsModel.h"

#include "SyncInfo.h"

SyncSettingsModel::SyncSettingsModel(QObject* parent):
    QAbstractListModel(parent),
    mSyncInfo(SyncInfo::instance())
{
    connect(mSyncInfo, &SyncInfo::syncStateChanged, this, &SyncSettingsModel::insertSync);
    connect(mSyncInfo, &SyncInfo::syncStatsUpdated, this, &SyncSettingsModel::updateSyncStats);
    connect(mSyncInfo, &SyncInfo::syncRemoved, this, &SyncSettingsModel::removeSync);

    mList = mSyncInfo->getSyncSettingsByType(mega::MegaSync::SyncType::TYPE_TWOWAY);
}

void SyncSettingsModel::insertSync(std::shared_ptr<SyncSettings> sync) {}

void SyncSettingsModel::updateSyncStats(std::shared_ptr<::mega::MegaSyncStats> stats) {}

void SyncSettingsModel::removeSync(std::shared_ptr<SyncSettings> sync) {}

QHash<int, QByteArray> SyncSettingsModel::roleNames() const
{
    return {{FolderRole, "folder"}, {StatusRole, "status"}};
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
            return sync->name(false, true);

        case StatusRole:
            return getState(sync);

        default:
            return {};
    }
}

QString SyncSettingsModel::getState(std::shared_ptr<SyncSettings> sync) const
{
    QString s;
    switch (sync->getRunState())
    {
        case ::mega::MegaSync::RUNSTATE_PENDING:
        case ::mega::MegaSync::RUNSTATE_LOADING:
            s = tr("Loading");
            break;
        case ::mega::MegaSync::RUNSTATE_SUSPENDED:
        {
            if (sync->getError())
            {
                s = tr("Stopped");
            }
            else
            {
                s = tr("Paused");
            }
            break;
        }
        case ::mega::MegaSync::RUNSTATE_DISABLED:
            s = tr("Disabled");
            break;
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
                    s = tr("Monitoring");
                }
            }
        }
    }
    return s;
}
