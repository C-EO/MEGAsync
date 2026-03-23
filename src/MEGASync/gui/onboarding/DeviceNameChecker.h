#ifndef DEVICE_NAME_CHECKER_H
#define DEVICE_NAME_CHECKER_H

#include "DeviceNames.h"
#include "megaapi.h"
#include "MyBackupsHandle.h"

#include <QObject>
#include <QSet>

#include <optional>

class DeviceNameChecker: public QObject
{
    Q_OBJECT

public:
    explicit DeviceNameChecker(QString deviceNameCandidate, QObject* parent = nullptr);

public slots:
    void process();
    void cancel();

signals:
    void deviceNameCheck(bool isValid);
    void finished();

private:
    mega::MegaApi* mMegaApi;
    std::shared_ptr<UserAttributes::MyBackupsHandle> mMyBackupsHandle;
    std::shared_ptr<UserAttributes::DeviceNames> mDeviceName;
    QSet<QString> mBackupDeviceNames;
    int mPendingRequests = 0;
    QString mDeviceNameCandidate;
    bool mFinished = false;

    void complete(std::optional<bool> isValid = std::nullopt);
    void fetchBackupDeviceNames();
    void updateReadyCondition();
    bool checkDeviceName(const QString& deviceName);
};

#endif
