#ifndef NODESELECTORSPECIALIZED_H
#define NODESELECTORSPECIALIZED_H

#include "FilePickerNodeSelector.h"
#include "NodeSelector.h"

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

private:
    void onOkButtonClicked() override;
    bool isFullSync();
};

class StreamNodeSelector: public FilePickerNodeSelector
{
    Q_OBJECT

public:
    explicit StreamNodeSelector(QWidget* parent = 0);

private:
    void onOkButtonClicked() override;
};

////////////////////
class CloudDriveNodeSelector: public NodeSelector
{
    Q_OBJECT

public:
    explicit CloudDriveNodeSelector(QWidget* parent = 0);

    void enableDragAndDrop(bool enable);

    static void sendStats();

protected:
    void specialisedTreeViewWidgetsCreated() override;

    bool searchHasOwnTab() const override
    {
        return true;
    }

    ClearTypes searchClearType() const override;

    void handleSearch(const QString& text) override;
    void handleSearchHidden() override;

    void configureCloudDriveWidget() override;
    void configureIncomingSharesWidget() override;
    void configureBackupsWidget() override;
    void configureRubbishWidget() override;
    void configureSearchWidget(TabType type) override;

    void configureSidebar() override;
    void configureSearchTool() override;
    void configureActionButtonsPlacement() override;
    void configureFooterVisibility() override;

    void onLanguageChangeEvent() override;

protected slots:
    void onCustomButtonClicked(uint id) override;
    void onItemsAboutToBeMoved(const QList<mega::MegaHandle>& handles, int type) override;
    void onItemsAboutToBeMovedFailed(const QList<mega::MegaHandle>& handles, int type) override;
    void onItemRequestsFinished(int actionType) override;
    void onItemsAboutToBeRestored(const QSet<mega::MegaHandle>& targetHandles) override;

    void onItemAboutToBeReplaced(mega::MegaHandle handle) override;

    void onItemsAboutToBeMerged(const QList<std::shared_ptr<NodeSelectorMergeInfo>>& mergesInfo,
                                int actionType) override;

    void onItemMergeFinished(mega::MegaHandle sourceHandle,
                             mega::MegaHandle targetHandle,
                             int actionType) override;

    void onItemsAboutToBeMergedFailed(
        const QList<std::shared_ptr<NodeSelectorMergeInfo>>& mergesInfo,
        int actionType) override;

    void onbShowSearchClicked();
    void onfShowSearchHidden();

private:
    void performMergeAction(const QList<std::shared_ptr<NodeSelectorMergeInfo>>& mergesInfo,
                            int actionType,
                            NodeSelector::IncreaseOrDecrease type);

    void onOkButtonClicked() override {}

    void checkMovingItems(const QList<mega::MegaHandle>& handles,
                          int moveType,
                          NodeSelector::IncreaseOrDecrease type);

    struct HandlesByTab
    {
        QList<mega::MegaHandle> cloudDriveNodes;
        QList<mega::MegaHandle> incomingSharedNodes;
    };

    HandlesByTab getTabs(const QList<mega::MegaHandle>& handles);
    void selectTabs(const HandlesByTab& tabsInfo);

    QWidget* mDragBackDrop;
};

//////////////////
class MoveBackupNodeSelector: public FilePickerNodeSelector
{
    Q_OBJECT

public:
    explicit MoveBackupNodeSelector(QWidget* parent = 0);

private:
    void onOkButtonClicked() override;
};

#endif // NODESELECTORSPECIALIZED_H
