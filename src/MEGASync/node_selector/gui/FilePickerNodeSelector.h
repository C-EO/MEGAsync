#ifndef FILEPICKERNODESELECTOR_H
#define FILEPICKERNODESELECTOR_H

#include "BannerWidget.h"
#include "NodeSelector.h"

class FilePickerNodeSelector: public NodeSelector
{
    Q_OBJECT

public:
    explicit FilePickerNodeSelector(SelectTypeSPtr selectType, QWidget* parent = nullptr);

protected:
    ClearTypes searchClearType() const override;
    void handleSearch(const QString& text) override;
    void handleSearchHidden() override;

    void refreshDestinationBreadcrumb() override;

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

    bool searchHasOwnTab() const override
    {
        return false;
    }

    bool initialShowLabelText() const override
    {
        return false;
    }

    void configureTypeSpecificColumns(NodeSelectorTreeViewWidget* widget) override;
    void configureSearchWidget(TabType type) override;
    void clearSearch() override;

    void configureSidebar() override;
    void configureHeader() override;

protected slots:
    void onItemsAboutToBeMoved(const QList<mega::MegaHandle>& handles, int actionType) override;
    void onItemsAboutToBeMovedFailed(const QList<mega::MegaHandle>& handles, int type) override;
    void onBreadcrumbClearRequested();

private:
    QStringList destinationBreadcrumbSegments() const;
    QStringList breadcrumbSegmentsForNode(NodeSelectorTreeViewWidget* wid,
                                          const std::shared_ptr<mega::MegaNode>& node) const;
    std::shared_ptr<mega::MegaNode>
        destinationNodeForBreadcrumb(NodeSelectorTreeViewWidget* wid) const;
};

#endif // FILEPICKERNODESELECTOR_H
