#ifndef SYNC_SETTINGS_MODEL_H
#define SYNC_SETTINGS_MODEL_H

#include "SyncController.h"
#include "SyncSettingsModelBase.h"

class SyncSettingsModel: public SyncSettingsModelBase
{
    Q_OBJECT

public:
    explicit SyncSettingsModel(QObject* parent = nullptr):
        SyncSettingsModelBase(mega::MegaSync::SyncType::TYPE_TWOWAY, parent)
    {
        connect(&SyncController::instance(),
                &SyncController::syncRemoveBegins,
                this,
                &SyncSettingsModel::onSyncRemoveBegins);

        connect(&SyncController::instance(),
                &SyncController::syncRemoveEnds,
                this,
                &SyncSettingsModel::onSyncRemoveEnds);
    }

    ~SyncSettingsModel() override = default;
};

#endif // SYNC_SETTINGS_MODEL_H
