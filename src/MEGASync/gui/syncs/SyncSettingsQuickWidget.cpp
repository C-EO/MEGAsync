#include "SyncSettingsQuickWidget.h"

#include "CreateRemoveSyncsManager.h"
#include "Platform.h"
#include "QmlManager.h"
#include "SyncSettingsModel.h"

SyncSettingsQuickWidget::SyncSettingsQuickWidget(QWidget* parent):
    MegaQuickWidget(parent)
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
