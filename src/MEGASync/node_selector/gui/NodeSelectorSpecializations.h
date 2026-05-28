#ifndef NODESELECTORSPECIALIZED_H
#define NODESELECTORSPECIALIZED_H

#include "FilePickerNodeSelectorSpecializations.h"
#include "NodeSelector.h"

////////////////////
class CloudDriveNodeSelector: public NodeSelector
{
    Q_OBJECT

public:
    explicit CloudDriveNodeSelector(QWidget* parent = 0);

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

    void configureTypeSpecificColumns(NodeSelectorTreeViewWidget* widget) override;
    void configureSearchWidget(TabType type) override;

    void configureSidebar() override;
    void configureHeader() override;
    void configureActionButtonsPlacement() override;
    void configureFooterVisibility() override;

    void refreshSearchResultCount() override;

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
#endif // NODESELECTORSPECIALIZED_H
