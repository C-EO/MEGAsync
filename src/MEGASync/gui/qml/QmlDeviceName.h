#ifndef QMLDEVICENAME_H
#define QMLDEVICENAME_H

#include "DeviceNames.h"

#include <QObject>
#include <QString>

#include <memory>

class QmlDeviceName: public QObject
{
    Q_OBJECT
    Q_PROPERTY(
        QString name MEMBER mName READ getDeviceName WRITE setDeviceName NOTIFY deviceNameChanged)

public:
    explicit QmlDeviceName(QObject* parent = nullptr);

    Q_INVOKABLE QString getDeviceName() const;
    Q_INVOKABLE void checkDeviceName(const QString& newName);
    Q_INVOKABLE void setDeviceName(const QString& newName);

signals:
    void deviceNameChanged();
    void deviceNameChecked(bool isValid);
    void deviceNameSetRequestCompleted();

private slots:
    void onDeviceNameReady();

private:
    bool isCandidateNameAvailable() const;

    QString mName;
    QString mCandidateName;
    bool mCheckDeviceNameRequested;
    bool mSetDeviceNameRequested;
    std::shared_ptr<UserAttributes::DeviceNames> mDeviceNamesRequest;
};

#endif // QMLDEVICENAME_H
