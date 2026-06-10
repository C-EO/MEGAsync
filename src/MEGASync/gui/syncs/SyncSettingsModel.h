#ifndef SYNC_SETTINGS_MODEL_H
#define SYNC_SETTINGS_MODEL_H

#include "SyncSettingsModelBase.h"

class SyncSettingsModel: public SyncSettingsModelBase
{
    Q_OBJECT

public:
    explicit SyncSettingsModel(QObject* parent = nullptr):
        SyncSettingsModelBase(mega::MegaSync::SyncType::TYPE_TWOWAY, parent)
    {}

    ~SyncSettingsModel() override = default;
};

#endif // SYNC_SETTINGS_MODEL_H
