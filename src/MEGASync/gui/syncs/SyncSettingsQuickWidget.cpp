#include "SyncSettingsQuickWidget.h"

SyncSettingsQuickWidget::SyncSettingsQuickWidget(QWidget* parent):
    MegaQuickWidget(parent)
{
    setResizeMode(QQuickWidget::SizeRootObjectToView);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setSource(QString::fromUtf8("qrc:/settings/SyncSettings.qml"));
}
