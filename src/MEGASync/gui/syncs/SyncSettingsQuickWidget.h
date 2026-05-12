#ifndef SYNC_SETTINGS_QUICK_WIDGET_H
#define SYNC_SETTINGS_QUICK_WIDGET_H

#include "MegaQuickWidget.h"
#include "SyncSettingsModel.h"

class SyncSettingsQuickWidget: public MegaQuickWidget
{
    Q_OBJECT

    Q_PROPERTY(bool automaticSyncIssueResolverEnabled READ getAutomaticSyncIssueResolverEnabled
                   WRITE setAutomaticSyncIssueResolverEnabled NOTIFY
                       automaticSyncIssueResolverEnabledChanged FINAL)

public:
    explicit SyncSettingsQuickWidget(QWidget* parent = nullptr);

    Q_INVOKABLE void openInMega(int index) const;
    Q_INVOKABLE void exploreLocalSync(const QString& localFolder) const;
    Q_INVOKABLE void addSync() const;

    bool getAutomaticSyncIssueResolverEnabled() const;
    void setAutomaticSyncIssueResolverEnabled(bool enable);

signals:
    void automaticSyncIssueResolverEnabledChanged();

private:
    bool mAutomaticSyncIssueResolverEnabled;
    SyncSettingsModel* mSyncModel;
};

#endif // SYNC_SETTINGS_QUICK_WIDGET_H
