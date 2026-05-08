#include "SyncSettingsQuickWidget.h"

#include "CreateRemoveSyncsManager.h"
#include "Platform.h"
#include "QmlManager.h"
#include "StalledIssuesModel.h"
#include "StatsEventHandler.h"
#include "SyncSettingsModel.h"

SyncSettingsQuickWidget::SyncSettingsQuickWidget(QWidget* parent):
    MegaQuickWidget(parent),
    mAutomaticSyncIssueResolverEnabled(Preferences::instance()->isStalledIssueSmartModeActivated())
{
    setResizeMode(QQuickWidget::SizeRootObjectToView);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    qmlRegisterType<SyncSettingsModel>("SyncSettingsModel", 1, 0, "SyncSettingsModel");

    QmlManager::instance()->setRootContextProperty(QStringLiteral("syncSettingsModel"),
                                                   new SyncSettingsModel(this));

    QmlManager::instance()->setRootContextProperty(QStringLiteral("syncSettings"), this);

    setSource(QString::fromUtf8("qrc:/settings/SyncSettings.qml"));
}

void SyncSettingsQuickWidget::exploreLocalSync(const QString& localFolder) const
{
    Platform::getInstance()->showInFolder(localFolder);
}

void SyncSettingsQuickWidget::addSync() const
{
    CreateRemoveSyncsManager::addSync(SyncInfo::SyncOrigin::SETTINGS_ORIGIN);
}

bool SyncSettingsQuickWidget::getAutomaticSyncIssueResolverEnabled() const
{
    return mAutomaticSyncIssueResolverEnabled;
}

void SyncSettingsQuickWidget::setAutomaticSyncIssueResolverEnabled(bool enable)
{
    if (mAutomaticSyncIssueResolverEnabled != enable)
    {
        if (enable)
        {
            Preferences::instance()->setStalledIssuesMode(
                Preferences::StalledIssuesModeType::Smart);
            MegaSyncApp->getStalledIssuesModel()->updateActiveStalledIssues();
            MegaSyncApp->getStatsEventHandler()->sendEvent(
                AppStatsEvents::EventType::SETTINGS_ISSUE_RESOLUTION_SMART);
        }
        else
        {
            Preferences::instance()->setStalledIssuesMode(
                Preferences::StalledIssuesModeType::Advance);
            MegaSyncApp->getStatsEventHandler()->sendEvent(
                AppStatsEvents::EventType::SETTINGS_ISSUE_RESOLUTION_ADVANCED);
        }

        mAutomaticSyncIssueResolverEnabled = enable;
        emit automaticSyncIssueResolverEnabledChanged();
    }
}