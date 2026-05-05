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
        FolderRole = Qt::UserRole + 1,
        StatusRole
    };

public:
    explicit SyncSettingsModel(QObject* parent = nullptr);
    virtual ~SyncSettingsModel() = default;

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;

private slots:
    void insertSync(std::shared_ptr<SyncSettings> sync);
    void updateSyncStats(std::shared_ptr<mega::MegaSyncStats> stats);
    void removeSync(std::shared_ptr<SyncSettings> sync);

private:
    SyncInfo* mSyncInfo;
    QList<std::shared_ptr<SyncSettings>> mList;

    QString getState(std::shared_ptr<SyncSettings> sync) const;
};

#endif // SYNC_SETTINGS_MODEL_H
