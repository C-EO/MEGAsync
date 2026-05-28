#ifndef FILEPICKERNODESELECTORSPECIALIZATIONS_H
#define FILEPICKERNODESELECTORSPECIALIZATIONS_H

#include "FilePickerNodeSelector.h"

class UploadNodeSelector: public FilePickerNodeSelector
{
    Q_OBJECT

public:
    explicit UploadNodeSelector(QWidget* parent = 0);

private:
    void onOkButtonClicked() override;
};

class DownloadNodeSelector: public FilePickerNodeSelector
{
    Q_OBJECT

public:
    explicit DownloadNodeSelector(QWidget* parent = 0);

private:
    void onOkButtonClicked() override;
};

class SyncNodeSelector: public FilePickerNodeSelector
{
    Q_OBJECT

public:
    explicit SyncNodeSelector(QWidget* parent = 0);

protected:
    QString destinationBreadcrumbEmptyText() override;

protected slots:
    void onModelModified() override;

private:
    void onOkButtonClicked() override;
    void refreshDestinationBreadcrumb() override;
    bool isFullSync();
    bool fullAccessInTopRootShares() const;
    bool enableFoldersInTopRootShares() const;
    bool incomingSharesTabIsEmpty() const;
    QString destinationTitleText() const override;
    DestinationBannerInfo destinationBannerInfo() const override;
};

class StreamNodeSelector: public FilePickerNodeSelector
{
    Q_OBJECT

public:
    explicit StreamNodeSelector(QWidget* parent = 0);

private:
    void onOkButtonClicked() override;
    QString destinationTitleText() const override;
    DestinationBannerInfo destinationBannerInfo() const override;
};

class MoveBackupNodeSelector: public FilePickerNodeSelector
{
    Q_OBJECT

public:
    explicit MoveBackupNodeSelector(QWidget* parent = 0);

private:
    void onOkButtonClicked() override;
};

#endif // FILEPICKERNODESELECTORSPECIALIZATIONS_H
