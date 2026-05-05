#include "SyncSettingsQuickWidget.h"

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

    setSource(QString::fromUtf8("qrc:/settings/SyncSettings.qml"));
}
