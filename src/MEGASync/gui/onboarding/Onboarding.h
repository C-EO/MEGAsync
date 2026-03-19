#ifndef ONBOARDING_H
#define ONBOARDING_H

#include "QmlDialogWrapper.h"

#include <QPointer>

class DeviceNameChecker;
class QThread;
class SyncsComponent;
class Onboarding: public QMLComponent
{
    Q_OBJECT

public:
    explicit Onboarding(QObject* parent = 0);
    ~Onboarding() override;
    QUrl getQmlUrl() override;

    Q_INVOKABLE void openPreferences(int tabIndex) const;
    Q_INVOKABLE void checkDeviceName(const QString& name);
    Q_INVOKABLE void showClosingButLoggingInWarningDialog() const;
    Q_INVOKABLE void showClosingButCreatingAccount() const;

signals:
    void accountBlocked(int errorCode);
    void logout();
    void deviceNameChecked(bool valid);

private:
    void stopDeviceNameCheck();

    std::unique_ptr<SyncsComponent> mSyncsComponent;
    QPointer<QThread> mDeviceNameThread;
    QPointer<DeviceNameChecker> mDeviceNameChecker;
};

#endif // ONBOARDING_H
