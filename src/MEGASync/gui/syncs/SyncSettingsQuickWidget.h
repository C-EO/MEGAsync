#ifndef SYNC_SETTINGS_QUICK_WIDGET_H
#define SYNC_SETTINGS_QUICK_WIDGET_H

#include "MegaQuickWidget.h"

class SyncSettingsQuickWidget: public MegaQuickWidget
{
    Q_OBJECT

public:
    explicit SyncSettingsQuickWidget(QWidget* parent = nullptr);

    Q_INVOKABLE void exploreLocalSync(const QString& localFolder) const;
    Q_INVOKABLE void addSync() const;
};

#endif // SYNC_SETTINGS_QUICK_WIDGET_H
