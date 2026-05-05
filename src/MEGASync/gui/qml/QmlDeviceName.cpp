#include "QmlDeviceName.h"

QmlDeviceName::QmlDeviceName(QObject* parent):
    QObject{parent},
    mCheckDeviceNameRequested(false),
    mSetDeviceNameRequested(false),
    mDeviceNamesRequest(UserAttributes::DeviceNames::requestDeviceNames())
{
    connect(mDeviceNamesRequest.get(),
            &UserAttributes::DeviceNames::attributeReady,
            this,
            &QmlDeviceName::onDeviceNameReady,
            Qt::QueuedConnection);

    if (mDeviceNamesRequest->isAttributeReady())
    {
        onDeviceNameReady();
    }
}

QString QmlDeviceName::getDeviceName() const
{
    return mName;
}

void QmlDeviceName::checkDeviceName(const QString& newName)
{
    mCheckDeviceNameRequested = true;
    mCandidateName = newName;

    if (mDeviceNamesRequest->isAttributeReady())
    {
        onDeviceNameReady();
    }
}

void QmlDeviceName::setDeviceName(const QString& newName)
{
    mSetDeviceNameRequested = true;

    if (mName == newName)
    {
        onDeviceNameReady();
    }
    else
    {
        mDeviceNamesRequest->setDeviceName(newName);
    }
}

void QmlDeviceName::onDeviceNameReady()
{
    auto newName = mDeviceNamesRequest->getDeviceName();
    if (mName != newName)
    {
        mName = newName;
        emit deviceNameChanged();
    }

    if (mCheckDeviceNameRequested)
    {
        mCheckDeviceNameRequested = false;
        emit deviceNameChecked(isCandidateNameAvailable());
    }

    // Emit only if this object is the source of the device name set
    if (mSetDeviceNameRequested)
    {
        mSetDeviceNameRequested = false;
        emit deviceNameSetRequestCompleted();
    }
}

bool QmlDeviceName::isCandidateNameAvailable() const
{
    return mDeviceNamesRequest->getDeviceName() == mCandidateName ||
           mDeviceNamesRequest->getDeviceNames().key(mCandidateName).isEmpty();
}