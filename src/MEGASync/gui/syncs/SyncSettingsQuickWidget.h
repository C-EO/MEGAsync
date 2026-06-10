#ifndef SYNC_SETTINGS_QUICK_WIDGET_H
#define SYNC_SETTINGS_QUICK_WIDGET_H

#include "SettingsQuickWidgetBase.h"

class SyncSettingsQuickWidget: public SettingsQuickWidgetBase
{
    Q_OBJECT

    Q_PROPERTY(bool automaticSyncIssueResolverEnabled READ getAutomaticSyncIssueResolverEnabled
                   WRITE setAutomaticSyncIssueResolverEnabled NOTIFY
                       automaticSyncIssueResolverEnabledChanged FINAL)

public:
    explicit SyncSettingsQuickWidget(QWidget* parent = nullptr);

    void addItem() const override;
    void remove(int index) const override;

    Q_INVOKABLE void restoreSyncedFolder(int index);

    bool getAutomaticSyncIssueResolverEnabled() const;
    void setAutomaticSyncIssueResolverEnabled(bool enable);

signals:
    void automaticSyncIssueResolverEnabledChanged();

private:
    bool mAutomaticSyncIssueResolverEnabled;
};

#endif // SYNC_SETTINGS_QUICK_WIDGET_H
