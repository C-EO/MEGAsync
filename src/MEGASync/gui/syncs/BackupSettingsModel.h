#ifndef BACKUP_SETTINGS_MODEL_H
#define BACKUP_SETTINGS_MODEL_H

#include "SyncController.h"
#include "SyncSettingsModelBase.h"

class BackupSettingsModel: public SyncSettingsModelBase
{
    Q_OBJECT

public:
    explicit BackupSettingsModel(QObject* parent = nullptr):
        SyncSettingsModelBase(mega::MegaSync::SyncType::TYPE_BACKUP, parent)
    {
        connect(&SyncController::instance(),
                &SyncController::syncRemoveBegins,
                this,
                &BackupSettingsModel::onSyncRemoveBegins);

        connect(&SyncController::instance(),
                &SyncController::syncRemoveEnds,
                this,
                &BackupSettingsModel::onSyncRemoveEnds);
    }

    ~BackupSettingsModel() override = default;
};

#endif // BACKUP_SETTINGS_MODEL_H
