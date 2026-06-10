#ifndef BACKUP_SETTINGS_QUICK_WIDGET_H
#define BACKUP_SETTINGS_QUICK_WIDGET_H

#include "megaapi.h"
#include "SettingsQuickWidgetBase.h"

#include <memory>

namespace UserAttributes
{
class MyBackupsHandle;
}

class BackupSettingsQuickWidget: public SettingsQuickWidgetBase
{
    Q_OBJECT
    Q_PROPERTY(
        QString backupFolderPath READ getBackupFolderPath NOTIFY backupFolderPathChanged FINAL)
    Q_PROPERTY(bool backupFolderAvailable READ isBackupFolderAvailable NOTIFY
                   backupFolderAvailableChanged FINAL)

public:
    explicit BackupSettingsQuickWidget(QWidget* parent = nullptr);

    void addItem() const override;
    void remove(int index) const override;

    Q_INVOKABLE void openBackupFolder() const;

    QString getBackupFolderPath() const;
    bool isBackupFolderAvailable() const;

signals:
    void backupFolderPathChanged();
    void backupFolderAvailableChanged();

private slots:
    void onMyBackupsFolderHandleSet(mega::MegaHandle handle);

private:
    std::shared_ptr<UserAttributes::MyBackupsHandle> mMyBackupsHandleRequest;
    QString mBackupFolderPath;
    bool mBackupFolderAvailable;
};

#endif // BACKUP_SETTINGS_QUICK_WIDGET_H
