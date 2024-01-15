#include "Backups.h"

#include "qml/QmlDialog.h"
#include "qml/AccountInfoData.h"

#include "onboarding/QmlDeviceName.h"
#include "onboarding/ChooseFolder.h"

#include "BackupsModel.h"

Backups::Backups(QObject *parent)
    : QMLComponent(parent)
{
    registerQmlModules();
}

QUrl Backups::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/backups/BackupsDialog.qml"));
}

QString Backups::contextName()
{
    return QString::fromUtf8("backupsAccess");
}

void Backups::registerQmlModules()
{
    qmlRegisterType<QmlDialog>("QmlDialog", 1, 0, "QmlDialog");
    qmlRegisterType<QmlDeviceName>("QmlDeviceName", 1, 0, "QmlDeviceName");
    qmlRegisterType<ChooseLocalFolder>("ChooseLocalFolder", 1, 0, "ChooseLocalFolder");
    qmlRegisterSingletonType<AccountInfoData>("AccountInfoData", 1, 0, "AccountInfoData", AccountInfoData::instance);

    qmlRegisterModule("Backups", 1, 0);
    qmlRegisterType<BackupsProxyModel>("BackupsProxyModel", 1, 0, "BackupsProxyModel");
    qmlRegisterUncreatableType<BackupsModel>("BackupsModel", 1, 0, "BackupErrorCode",
                                             QString::fromUtf8("Cannot register BackupsModel::BackupErrorCode in QML"));
}
