#ifndef BACKUP_SETTINGS_MODEL_H
#define BACKUP_SETTINGS_MODEL_H

#include "SyncSettingsModelBase.h"

class BackupSettingsModel: public SyncSettingsModelBase
{
    Q_OBJECT

public:
    explicit BackupSettingsModel(QObject* parent = nullptr):
        SyncSettingsModelBase(mega::MegaSync::SyncType::TYPE_BACKUP, parent)
    {}

    ~BackupSettingsModel() override = default;
};

#endif // BACKUP_SETTINGS_MODEL_H
