#include "MessageDialogComponent.h"

static bool qmlRegistrationDone = false;

MessageDialogComponent::MessageDialogComponent(QObject* parent, QPointer<MessageDialogData> data):
    QMLComponent(parent),
    mData(data)
{
    registerQmlModules();

    QmlManager::instance()->setRootContextProperty(mData.data());
}

QUrl MessageDialogComponent::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/messageDialogs/MessageDialog.qml"));
}

QList<QObject*> MessageDialogComponent::getInstancesFromContext()
{
    QList<QObject*> instances;
    QObject* messageDialogObject(qobject_cast<QObject*>(mData.data()));
    if (messageDialogObject)
    {
        instances.append(messageDialogObject);
    }
    return instances;
}

void MessageDialogComponent::registerQmlModules()
{
    if (!qmlRegistrationDone)
    {
        qmlRegisterUncreatableMetaObject(
            MessageDialogButtonInfo::staticMetaObject,
            "MessageDialogButtonInfo",
            1,
            0,
            "MessageDialogButtonInfo",
            QString::fromLatin1("MessageDialogButtonInfo can only be used for the enum values"));

        // SNC-6567 (Phase 3): MessageDialogTextInfo is now a QObject (not a
        // Q_GADGET), so registration uses qmlRegisterUncreatableType. QML can
        // still reference the TextFormat enum (`MessageDialogTextInfo.TextFormat.RICH`)
        // and read property values from instances handed in via context
        // properties, but cannot instantiate one from QML.
        qmlRegisterUncreatableType<MessageDialogTextInfo>(
            "MessageDialogTextInfo",
            1,
            0,
            "MessageDialogTextInfo",
            QString::fromLatin1(
                "MessageDialogTextInfo cannot be instantiated from QML; use the enum values only"));

        qmlRegisterUncreatableType<MessageDialogData>(
            "MessageDialogData",
            1,
            0,
            "MessageDialogData",
            QString::fromLatin1("MessageDialogDataType can only be used for the enum values"));

        qmlRegistrationDone = true;
    }
}

void MessageDialogComponent::buttonClicked(int type)
{
    mData->buttonClicked(static_cast<QMessageBox::StandardButton>(type));
}

void MessageDialogComponent::setChecked(bool checked)
{
    mData->setCheckboxChecked(checked);
}
