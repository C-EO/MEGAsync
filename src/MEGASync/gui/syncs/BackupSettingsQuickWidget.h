#ifndef BACKUP_SETTINGS_QUICK_WIDGET_H
#define BACKUP_SETTINGS_QUICK_WIDGET_H

#include "BackupSettingsModel.h"
#include "MegaQuickWidget.h"

#include <memory>

namespace UserAttributes
{
class MyBackupsHandle;
}

class BackupSettingsQuickWidget: public MegaQuickWidget
{
    Q_OBJECT
    Q_PROPERTY(
        QString backupFolderPath READ getBackupFolderPath NOTIFY backupFolderPathChanged FINAL)
    Q_PROPERTY(bool backupFolderAvailable READ isBackupFolderAvailable NOTIFY
                   backupFolderAvailableChanged FINAL)

public:
    explicit BackupSettingsQuickWidget(QWidget* parent = nullptr);

    Q_INVOKABLE void openInMega(int index) const;
    Q_INVOKABLE void exploreLocalBackup(const QString& localFolder) const;
    Q_INVOKABLE void addBackup() const;
    Q_INVOKABLE void pauseBackup(int index) const;
    Q_INVOKABLE void resumeBackup(int index) const;
    Q_INVOKABLE void openExclusionsDialog(int index) const;
    Q_INVOKABLE void rescan(int index) const;
    Q_INVOKABLE void reboot(int index) const;
    Q_INVOKABLE void remove(int index) const;
    Q_INVOKABLE void removeNonConfirmation(int index) const;
    Q_INVOKABLE void openOverQuotaDialog() const;
    Q_INVOKABLE void openBackupFolder() const;
    Q_INVOKABLE void sortModelByName(bool ascending = true);
    Q_INVOKABLE void sortModelByStatus(bool ascending = true);

    QString getBackupFolderPath() const;
    bool isBackupFolderAvailable() const;

signals:
    void backupFolderPathChanged();
    void backupFolderAvailableChanged();

private slots:
    void onMyBackupsFolderHandleSet(mega::MegaHandle handle);

private:
    BackupSettingsModel* mBackupModel;
    std::shared_ptr<UserAttributes::MyBackupsHandle> mMyBackupsHandleRequest;
    QString mBackupFolderPath;
    bool mBackupFolderAvailable;
};

#endif // BACKUP_SETTINGS_QUICK_WIDGET_H
