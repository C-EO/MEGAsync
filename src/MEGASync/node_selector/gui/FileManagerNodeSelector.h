#ifndef FILEMANAGERNODESELECTOR_H
#define FILEMANAGERNODESELECTOR_H

#include "NodeSelector.h"

////////////////////
class FileManagerNodeSelector: public NodeSelector
{
    Q_OBJECT

public:
    explicit FileManagerNodeSelector(QWidget* parent = 0);

    static void sendStats();

protected:
    void specialisedTreeViewWidgetsCreated() override;

    void configureTypeSpecificColumns(NodeSelectorTreeViewWidget* widget) override;

    void configureSidebar() override;
    void configureHeader() override;
    void configureActionButtonsPlacement() override;
    void configureFooter() override;

    void refreshSearchResultCount() override;
    void refreshNavigationBreadcrumb() override;
    void onNodesRenamed(const QList<mega::MegaHandle>& handles) override;

    void onLanguageChangeEvent() override;

    void applyNewFolderSelection(NodeSelectorTreeViewWidget* sourceWidget,
                                 mega::MegaNode* newNode) override;

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

private slots:
    void onNavigationBreadcrumbSegmentActivated(int segmentIndex);
    void onNavigationBreadcrumbMenuRequested(const QPoint& globalPos);

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
#endif // FILEMANAGERNODESELECTOR_H
