#ifndef SYNC_SETTINGS_MODEL_H
#define SYNC_SETTINGS_MODEL_H

#include "megaapi.h"

#include <QAbstractListModel>

#include <memory>

class SyncInfo;
class SyncSettings;

class SyncSettingsModel: public QAbstractListModel
{
    Q_OBJECT

    enum Role
    {
        NameRole = Qt::UserRole + 1,
        FolderRole,
        StatusRole,
        StatusId,
        ErrorMessage,
        ErrorId
    };

public:
    explicit SyncSettingsModel(QObject* parent = nullptr);
    virtual ~SyncSettingsModel() = default;

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    std::shared_ptr<SyncSettings> getSync(int index) const;

    enum SyncStates
    {
        PENDING,
        LOADING,
        SUSPENDED,
        ERROR,
        SCANNING,
        SYNCING,
        SYNCED,
        DISABLED
    };

    Q_ENUM(SyncStates)

private slots:
    void insertSync(std::shared_ptr<SyncSettings> sync);
    void updateSyncStats(std::shared_ptr<mega::MegaSyncStats> stats);
    void removeSync(std::shared_ptr<SyncSettings> sync);

private:
    SyncInfo* mSyncInfo;
    QList<std::shared_ptr<SyncSettings>> mList;

    SyncStates getState(std::shared_ptr<SyncSettings> sync) const;
    void sendDataChanged(int row);
    QString getErrorMessage(std::shared_ptr<SyncSettings> sync) const;
};

#endif // SYNC_SETTINGS_MODEL_H
