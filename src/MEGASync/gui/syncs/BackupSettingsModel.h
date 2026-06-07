#ifndef BACKUP_SETTINGS_MODEL_H
#define BACKUP_SETTINGS_MODEL_H

#include "megaapi.h"

#include <QAbstractListModel>
#include <QCollator>

#include <memory>

class SyncInfo;
class SyncSettings;

class BackupSettingsModel: public QAbstractListModel
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
    explicit BackupSettingsModel(QObject* parent = nullptr);
    virtual ~BackupSettingsModel() = default;

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    std::shared_ptr<SyncSettings> getBackup(int index) const;
    void sortByName(bool ascending = true);
    void sortByStatus(bool ascending = true);

    enum BackupStates
    {
        PENDING,
        LOADING,
        SUSPENDED,
        FAIL,
        SCANNING,
        BACKING_UP,
        BACKED_UP
    };

    Q_ENUM(BackupStates)

private slots:
    void insertBackup(std::shared_ptr<SyncSettings> sync);
    void updateBackupStats(std::shared_ptr<mega::MegaSyncStats> stats);
    void removeBackup(std::shared_ptr<SyncSettings> sync);

private:
    SyncInfo* mSyncInfo;
    QList<std::shared_ptr<SyncSettings>> mList;
    bool mSortAscending = true;
    QCollator mQCollator;

    BackupStates getState(std::shared_ptr<SyncSettings> sync) const;
    void sendDataChanged(int row);
    QString getErrorMessage(std::shared_ptr<SyncSettings> sync) const;
};

#endif // BACKUP_SETTINGS_MODEL_H
