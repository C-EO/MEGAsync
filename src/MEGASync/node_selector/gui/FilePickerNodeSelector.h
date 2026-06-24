#ifndef FILEPICKERNODESELECTOR_H
#define FILEPICKERNODESELECTOR_H

#include "BannerWidget.h"
#include "NodeSelector.h"
#include "NodeSelectorBreadcrumbSegment.h"

class FilePickerNodeSelector: public NodeSelector
{
    Q_OBJECT

public:
    explicit FilePickerNodeSelector(SelectTypeSPtr selectType, QWidget* parent = nullptr);

protected:
    void refreshDestinationBreadcrumb() override;
    void onNodesRenamed(const QList<mega::MegaHandle>& handles) override;
    // The file picker only shows the current top-root name, so it uses a read-only label instead
    // of the full NavigationBreadcrumb.
    void refreshNavigationBreadcrumb() override;

    virtual QString destinationBreadcrumbEmptyText()
    {
        return QString();
    }

    bool shouldClearSelectionOnBackgroundClick(const QPoint& pos) const override;

    void onLanguageChangeEvent() override;
    virtual QString destinationTitleText() const;

    struct DestinationBannerInfo
    {
        BannerWidget::Type type{BannerWidget::Type::NONE};
        QString text;
    };

    virtual DestinationBannerInfo destinationBannerInfo() const;

    bool initialShowLabelText() const override
    {
        return false;
    }

    void configureTypeSpecificColumns(NodeSelectorTreeViewWidget* widget) override;

    void configureSidebar() override;
    void configureHeader() override;

    std::shared_ptr<mega::MegaNode>
        getNewFolderParentNode(NodeSelectorTreeViewWidget* sourceWidget) const override;

    void applyNewFolderSelection(NodeSelectorTreeViewWidget* sourceWidget,
                                 mega::MegaNode* newNode) override;

protected slots:
    void onItemsAboutToBeMoved(const QList<mega::MegaHandle>& handles, int actionType) override;
    void onItemsAboutToBeMovedFailed(const QList<mega::MegaHandle>& handles, int type) override;
    void onBreadcrumbClearRequested();

private:
    QList<NodeSelectorBreadcrumbSegment> destinationBreadcrumbSegments() const;
    QList<NodeSelectorBreadcrumbSegment>
        breadcrumbSegmentsForNode(NodeSelectorTreeViewWidget* wid,
                                  const std::shared_ptr<mega::MegaNode>& node) const;
    std::shared_ptr<mega::MegaNode>
        destinationNodeForBreadcrumb(NodeSelectorTreeViewWidget* wid) const;
};

#endif // FILEPICKERNODESELECTOR_H
