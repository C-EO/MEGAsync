#ifndef SYNC_SETTINGS_QUICK_WIDGET_H
#define SYNC_SETTINGS_QUICK_WIDGET_H

#include "MegaQuickWidget.h"

class SyncSettingsQuickWidget: public MegaQuickWidget
{
    Q_OBJECT

public:
    explicit SyncSettingsQuickWidget(QWidget* parent = nullptr);
};

#endif // SYNC_SETTINGS_QUICK_WIDGET_H
