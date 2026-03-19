#include "DeviceNameChecker.h"

#include "MegaApplication.h"
#include "MyBackupsHandle.h"
#include "Utilities.h"

const static int INITIAL_PENDING_REQUEST = 2;

DeviceNameChecker::DeviceNameChecker(QString deviceNameCandidate, QObject* parent):
    QObject(parent),
    mMegaApi(MegaSyncApp->getMegaApi()),
    mDeviceNameCandidate(deviceNameCandidate)
{}

void DeviceNameChecker::process()
{
    if (mFinished)
    {
        emit finished();
        return;
    }

    mMyBackupsHandle = UserAttributes::MyBackupsHandle::requestMyBackupsHandle();
    mDeviceName = UserAttributes::DeviceNames::requestDeviceNames();

    mPendingRequests = INITIAL_PENDING_REQUEST;
    mBackupDeviceNames.clear();

    if (mDeviceName->isAttributeReady())
    {
        updateReadyCondition();
    }
    else
    {
        connect(mDeviceName.get(),
                &UserAttributes::DeviceNames::attributeReady,
                this,
                &DeviceNameChecker::updateReadyCondition);
    }

    if (mMyBackupsHandle->isAttributeReady())
    {
        fetchBackupDeviceNames();
    }
    else
    {
        connect(mMyBackupsHandle.get(),
                &UserAttributes::MyBackupsHandle::attributeReady,
                this,
                &DeviceNameChecker::fetchBackupDeviceNames,
                Qt::QueuedConnection);
    }
}

void DeviceNameChecker::fetchBackupDeviceNames()
{
    if (mFinished)
    {
        return;
    }

    if (mMyBackupsHandle->isAttributeReady())
    {
        auto backupHandle = mMyBackupsHandle->getMyBackupsHandle();
        if (backupHandle != mega::INVALID_HANDLE)
        {
            const std::unique_ptr<mega::MegaNode> backupFolder(
                mMegaApi->getNodeByHandle(backupHandle));
            const std::unique_ptr<mega::MegaNodeList> nodeList(
                mMegaApi->getChildren(backupFolder.get()));

            for (int nodeIndex = 0; nodeIndex < nodeList->size(); ++nodeIndex)
            {
                auto* node = nodeList->get(nodeIndex);
                if (node != nullptr && node->getType() == mega::MegaNode::TYPE_FOLDER)
                {
                    static const auto deviceId = QString::fromUtf8(mMegaApi->getDeviceId());
                    const auto nodeDeviceId = QString::fromUtf8(node->getDeviceId());
                    if (nodeDeviceId == deviceId)
                    {
                        // Only list folders for different device ids, to allow re-using a folder
                        continue;
                    }
                    mBackupDeviceNames << QString::fromUtf8(node->getName());
                }
            }
        }
    }

    updateReadyCondition();
}

void DeviceNameChecker::updateReadyCondition()
{
    if (mFinished || mPendingRequests <= 0)
    {
        return;
    }

    --mPendingRequests;
    if (mPendingRequests == 0)
    {
        complete(checkDeviceName(mDeviceNameCandidate));
    }
}

bool DeviceNameChecker::checkDeviceName(const QString& deviceName)
{
    return mDeviceName->getDeviceName() == deviceName ||
           (mDeviceName->getDeviceNames().key(deviceName).isEmpty() &&
            !mBackupDeviceNames.contains(deviceName));
}

void DeviceNameChecker::cancel()
{
    complete();
}

void DeviceNameChecker::complete(std::optional<bool> isValid)
{
    if (mFinished)
    {
        return;
    }

    mFinished = true;
    if (mDeviceName)
    {
        QObject::disconnect(mDeviceName.get(), nullptr, this, nullptr);
    }
    if (mMyBackupsHandle)
    {
        QObject::disconnect(mMyBackupsHandle.get(), nullptr, this, nullptr);
    }

    if (isValid.has_value())
    {
        emit deviceNameCheck(*isValid);
    }
    emit finished();
}
