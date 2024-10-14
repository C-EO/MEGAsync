#include "SyncModel.h"

#include "Utilities.h"

#include <algorithm>
#include <QCoreApplication>
#include <QDate>

SyncModel::SyncModel(QObject* parent):
    QAbstractListModel(parent)
{}

QHash<int, QByteArray> SyncModel::roleNames() const
{
    static QHash<int, QByteArray> roles{
        {TYPE,          "type"        },
        {NAME,          "name"        },
        {SIZE,          "size"        },
        {DATE_MODIFIED, "dateModified"},
        {STATUS,        "status"      }
    };
    return roles;
}

void SyncModel::add(const QmlSyncData& newSync)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    mSyncObjects.append(newSync);
    endInsertRows();
}

void SyncModel::addOrUpdate(const QmlSyncData& newSync)
{
    auto row = findRowByHandle(newSync.syncID);
    if (!row.has_value())
    {
        add(newSync);
    }
    else
    {
        mSyncObjects[row.value()].updateFields(newSync);
        const QModelIndex modelIndex = QAbstractListModel::index(row.value());
        emit dataChanged(modelIndex, modelIndex);
    }
}

void SyncModel::remove(mega::MegaHandle handle)
{
    auto row = findRowByHandle(handle);
    if (!row.has_value())
        return;

    if (row.has_value())
    {
        beginRemoveRows(QModelIndex(), row.value(), row.value());
        mSyncObjects.removeAt(row.value());
        endRemoveRows();
    }
}

void SyncModel::clear()
{
    mSyncObjects.clear();
}

std::optional<int> SyncModel::findRowByHandle(mega::MegaHandle handle) const
{
    auto finder = [handle](const QmlSyncData& obj)
    {
        return obj.syncID == handle;
    };
    auto itSyncObj = std::find_if(mSyncObjects.begin(), mSyncObjects.end(), finder);
    if (itSyncObj != mSyncObjects.end())
    {
        return std::distance(mSyncObjects.begin(), itSyncObj);
    }
    return std::nullopt;
}

SyncStatus::Value SyncModel::computeDeviceStatus() const
{
    SyncStatus::Value deviceStatus = SyncStatus::UP_TO_DATE;
    for (const auto& sync: mSyncObjects)
    {
        const SyncStatus::Value currentStatus = sync.status;
        if (deviceStatus < currentStatus)
        {
            deviceStatus = currentStatus;
        }
    }
    return deviceStatus;
}

qint64 SyncModel::computeTotalSize() const
{
    const qint64 startValue = 0;
    auto accumulator = [](qint64 total, const auto& sync) {
        return total + sync.size;
    };
    return std::accumulate(mSyncObjects.begin(), mSyncObjects.end(), startValue, accumulator);
}

void SyncModel::setStatus(mega::MegaHandle handle, const SyncStatus::Value status)
{
    auto row = findRowByHandle(handle);
    if (!row.has_value())
    {
        mSyncObjects[row.value()].status = status;

        const QModelIndex modelIndex = index(row.value());
        emit dataChanged(modelIndex, modelIndex);
    }
}

bool SyncModel::hasUpdatingStatus() const
{
    auto finder = [](const QmlSyncData& obj) {
        return obj.status == SyncStatus::UPDATING;
    };
    return std::any_of(mSyncObjects.begin(), mSyncObjects.end(), finder);
}

int SyncModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return mSyncObjects.size();
}

int SyncModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 5;
}

QVariant SyncModel::data(const QModelIndex& index, int role) const
{
    QVariant result;
    const auto row = index.row();
    const auto coulumn = index.column();
    if (row >= rowCount() || coulumn >= columnCount())
    {
        return result;
    }

    switch (role)
    {
        case NAME:
            result = getName(row);
            break;
        case TYPE:
            result = getType(row);
            break;
        case SIZE:
            result = getSize(row);
            break;
        case DATE_MODIFIED:
            result = getDateModified(row);
            break;
        case STATUS:
            result = getStatus(row);
            break;

        default:
            break;
    }
    return result;
}

QString SyncModel::getName(int row) const
{
    return mSyncObjects[row].name;
}

QString SyncModel::getSize(int row) const
{
    return Utilities::getSizeString(mSyncObjects[row].size);
}

QmlSyncType::Type SyncModel::getType(int row) const
{
    return mSyncObjects[row].type;
}

QDate SyncModel::getDateModified(int row) const
{
    return mSyncObjects[row].dateModified.date();
}

SyncStatus::Value SyncModel::getStatus(int row) const
{
    return mSyncObjects[row].status;
}

std::optional<mega::MegaHandle> SyncModel::getHandle(int row) const
{
    if (row < 0 || row >= mSyncObjects.size())
    {
        return {};
    }
    return mSyncObjects[row].nodeHandle;
}

QString SyncModel::getLocalFolder(int row) const
{
    if (row < 0 || row >= mSyncObjects.size())
    {
        return {};
    }
    return mSyncObjects[row].localFolder;
}

std::optional<mega::MegaHandle> SyncModel::getSyncID(int row) const
{
    if (row < 0 || row >= mSyncObjects.size())
    {
        return {};
    }
    return mSyncObjects[row].syncID;
}
