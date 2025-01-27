#ifndef NODESELECTORSPECIALIZED_H
#define NODESELECTORSPECIALIZED_H

#include "NodeSelector.h"

class UploadNodeSelector : public NodeSelector
{
    Q_OBJECT

public:
    explicit UploadNodeSelector(QWidget *parent = 0);

protected:
    void createSpecialisedWidgets() override;

private:
    void checkSelection() override;
};

class DownloadNodeSelector : public NodeSelector
{
    Q_OBJECT

public:
    explicit DownloadNodeSelector(QWidget *parent = 0);

protected:
    void createSpecialisedWidgets() override;

private:
    void checkSelection() override;
};

class SyncNodeSelector : public NodeSelector
{
    Q_OBJECT

public:
    explicit SyncNodeSelector(QWidget *parent = 0);

protected:
    void createSpecialisedWidgets() override;

private:
    void checkSelection() override;
    bool isFullSync();
};

class StreamNodeSelector : public NodeSelector
{
    Q_OBJECT

public:
    explicit StreamNodeSelector(QWidget *parent = 0);

protected:
    void createSpecialisedWidgets() override;

private:
    void checkSelection() override;
};

////////////////////
class CloudDriveNodeSelector : public NodeSelector
{
    Q_OBJECT

public:
    explicit CloudDriveNodeSelector(QWidget *parent = 0);

    void enableDragAndDrop(bool enable);

protected:
    void createSpecialisedWidgets() override;
    void doCustomConnections(NodeSelectorTreeViewWidget* item) override;

protected slots:
    void onCustomBottomButtonClicked(uint id) override;
    void onItemsAboutToBeMoved(const QList<mega::MegaHandle>& handles) override;

private slots:
    void onItemsRestoreRequested(const QList<mega::MegaHandle>& handles);
    void onItemsDeleteRequested(const QList<mega::MegaHandle>& handles);

private:
    void checkSelection() override {}

    QWidget* mDragBackDrop;
    bool mRestoreRequested;
};

//////////////////
class MoveBackupNodeSelector : public NodeSelector
{    
    Q_OBJECT
public:

    explicit MoveBackupNodeSelector(QWidget *parent = 0);

protected:
    void createSpecialisedWidgets() override;

private:
    void checkSelection() override;
};

#endif // NODESELECTORSPECIALIZED_H
