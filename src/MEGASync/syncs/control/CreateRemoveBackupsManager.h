#ifndef CREATEREMOVEBACKUPSMANAGER_H
#define CREATEREMOVEBACKUPSMANAGER_H

#include <QObject>
#include <QPointer>

#include <memory>

class SyncSettings;

class CreateRemoveBackupsManager: public QObject
{
    Q_OBJECT

public:
    CreateRemoveBackupsManager() = default;
    ~CreateRemoveBackupsManager() = default;

    static const CreateRemoveBackupsManager*
        addBackup(bool comesFromSettings, const QStringList& localFolders = QStringList());
    static const CreateRemoveBackupsManager* removeBackup(std::shared_ptr<SyncSettings> backup,
                                                          QWidget* parent);

    bool isBackupsDialogOpen() const;

private:
    void performAddBackup(const QStringList& localFolders, bool comesFromSettings);
    void performRemoveBackup(std::shared_ptr<SyncSettings> backup, QWidget* parent);
};

#endif // CREATEREMOVEBACKUPSMANAGER_H
