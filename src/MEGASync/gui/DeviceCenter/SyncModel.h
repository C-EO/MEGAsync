#ifndef SYNC_MODEL_H
#define SYNC_MODEL_H

#include "QmlSyncData.h"

#include <megaapi.h>
#include <QAbstractListModel>

class SyncModel: public QAbstractListModel
{
    Q_OBJECT

public:
    enum SyncModelRole
    {
        TYPE = Qt::UserRole + 1,
        NAME,
        SIZE,
        DATE_ADDED,
        DATE_MODIFIED,
        STATUS
    };

    enum SyncType
    {
        SYNC,
        BACKUP
    };
    Q_ENUM(SyncType)

    explicit SyncModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void add(const QmlSyncData& newSync);
    void addOrUpdate(const QmlSyncData& newSync);
    void remove(mega::MegaHandle handle);
    void clear();
    int findRowByHandle(mega::MegaHandle handle) const;
    SyncStatus::Value computeDeviceStatus() const;
    qint64 computeTotalSize() const;

    void setStatus(mega::MegaHandle handle, const SyncStatus::Value status);

public:
    QString getName(int row) const;
    QString getSize(int row) const;
    SyncType getType(int row) const;
    QDate getDateAdded(int row) const;
    QDate getDateModified(int row) const;
    SyncStatus::Value getStatus(int row) const;

    QList<QmlSyncData> mSyncObjects;
};

#endif // SYNC_MODEL_H
