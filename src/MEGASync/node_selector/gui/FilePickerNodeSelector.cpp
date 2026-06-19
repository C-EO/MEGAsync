#include "FilePickerNodeSelector.h"

#include "DestinationBreadcrumb.h"
#include "megaapi.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorProxyModel.h"
#include "NodeSelectorTreeViewWidgetSpecializations.h"
#include "ui_NodeSelector.h"

#include <QCoreApplication>

FilePickerNodeSelector::FilePickerNodeSelector(SelectTypeSPtr selectType, QWidget* parent):
    NodeSelector(selectType, parent)
{
    connect(ui->destinationBreadcrumb,
            &DestinationBreadcrumb::clearRequested,
            this,
            &FilePickerNodeSelector::onBreadcrumbClearRequested);
    connect(ui->destinationBreadcrumb,
            &DestinationBreadcrumb::refreshNeeded,
            this,
            &FilePickerNodeSelector::refreshDestinationBreadcrumb);
}

void FilePickerNodeSelector::onNodesRenamed(const QList<mega::MegaHandle>& handles)
{
    ui->destinationBreadcrumb->onNodesRenamed(handles);
}

void FilePickerNodeSelector::refreshDestinationBreadcrumb()
{
    const bool shouldShowPath = mSelectType && mSelectType->showsDestinationBreadcrumb() &&
                                selectedSearchChipTreeViewWidget();
    const auto banner = destinationBannerInfo();

    const bool shouldShowBanner = !banner.text.isEmpty() && banner.type != BannerWidget::Type::NONE;

    // Mutually exclusive: banner wins over breadcrumb when both would apply.
    if (shouldShowBanner)
    {
        ui->destinationBanner->setType(banner.type);
        ui->destinationBanner->setTitle(banner.text);
        ui->destinationBanner->setVisible(true);
        ui->destinationBreadcrumb->setVisible(false);
        ui->destinationBreadcrumb->setSegments({});
        return;
    }

    ui->destinationBanner->setVisible(false);

    if (!shouldShowPath)
    {
        ui->destinationBreadcrumb->setVisible(false);
        ui->destinationBreadcrumb->setSegments({});
        return;
    }

    ui->destinationBreadcrumb->setVisible(true);
    auto path{destinationBreadcrumbSegments()};
    if (path.isEmpty())
    {
        ui->destinationBreadcrumb->setSegments(
            {{mega::INVALID_HANDLE, destinationBreadcrumbEmptyText()}});
    }
    else
    {
        ui->destinationBreadcrumb->setSegments(path);
    }
    ui->destinationBreadcrumb->setTitleText(destinationTitleText());
}

bool FilePickerNodeSelector::shouldClearSelectionOnBackgroundClick(const QPoint& pos) const
{
    const bool keepSelectionOnRightPaneClick = mSelectType &&
                                               mSelectType->showsDestinationBreadcrumb() &&
                                               ui->wRightPaneNS->geometry().contains(pos);

    return !keepSelectionOnRightPaneClick;
}

void FilePickerNodeSelector::onLanguageChangeEvent()
{
    refreshDestinationBreadcrumb();
}

void FilePickerNodeSelector::onItemsAboutToBeMoved(const QList<mega::MegaHandle>& handles, int)
{
    performItemsToBeMoved(handles, IncreaseOrDecrease::INCREASE, true, true);
}

void FilePickerNodeSelector::onItemsAboutToBeMovedFailed(const QList<mega::MegaHandle>& handles,
                                                         int)
{
    performItemsToBeMoved(handles, IncreaseOrDecrease::DECREASE, true, true);
}

void FilePickerNodeSelector::onBreadcrumbClearRequested()
{
    // If there is no selection, move to top root indexpo
    if (auto* wid = getCurrentTreeViewWidget())
    {
        if (!wid->clearSelection())
        {
            wid->moveToTopRootIndex();
        }
    }
}

void FilePickerNodeSelector::configureTypeSpecificColumns(NodeSelectorTreeViewWidget* widget)
{
    if (!widget)
    {
        return;
    }

    widget->setColumnHidden(NodeSelectorModel::Column::ADDED_DATE, true);
    widget->setColumnHidden(NodeSelectorModel::Column::LAST_MODIFIED_DATE, true);
}

void FilePickerNodeSelector::configureSidebar()
{
    static constexpr int COLLAPSED_SIDEBAR_WIDTH = 60;
    static constexpr int COLLAPSED_TAB_SIZE = 36;
    static constexpr int COLLAPSED_ICON_SIZE = 20;

    ui->wLeftPaneNS->setMinimumWidth(COLLAPSED_SIDEBAR_WIDTH);
    ui->wLeftPaneNS->setMaximumWidth(COLLAPSED_SIDEBAR_WIDTH);

    const auto collapseTab = [](TabSelector* tab)
    {
        if (!tab)
        {
            return;
        }
        tab->setIconOnly(true);
        tab->setProperty("class", QLatin1String("sidebar-icononly"));
        tab->setFixedSize(COLLAPSED_TAB_SIZE, COLLAPSED_TAB_SIZE);
        tab->setIconSize(QSize(COLLAPSED_ICON_SIZE, COLLAPSED_ICON_SIZE));
        tab->style()->unpolish(tab);
        tab->style()->polish(tab);
    };

    collapseTab(ui->fCloudDrive);
    collapseTab(ui->fIncomingShares);
    collapseTab(ui->fBackups);
    collapseTab(ui->fRubbish);

    const auto applyHeaderStyle = [](TokenizableButton* btn)
    {
        if (!btn)
        {
            return;
        }
        btn->setIcon(QIcon());
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
    };

    for (auto* btn: {ui->bUpload, ui->bNewFolder, ui->bClearRubbish})
    {
        applyHeaderStyle(btn);
    }

    // The account info widget is only shown in the file manager (Cloud Drive).
    ui->wAccountInfo->setVisible(false);
}

void FilePickerNodeSelector::configureHeader()
{
    ui->leSearchTool->setMode(SearchLineEdit::Mode::ALWAYS_EXPANDED);

    static constexpr int SEARCH_LINE_EDIT_FIXED_WIDTH = 188;
    static constexpr int SEARCH_LINE_EDIT_FIXED_HEIGHT = 40;

    ui->leSearchTool->setFixedSize(SEARCH_LINE_EDIT_FIXED_WIDTH, SEARCH_LINE_EDIT_FIXED_HEIGHT);
    ui->actionButtonsContainer->setVisible(false);
    ui->navigationBreadcrumb->setVisible(false);
    ui->lNavigationRoot->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    if (auto* topRowLayout = ui->headerLayout)
    {
        ui->headerContainerLayout->removeWidget(ui->lNavigationRoot);
        topRowLayout->insertWidget(0, ui->lNavigationRoot, 0, Qt::AlignLeft | Qt::AlignTop);
    }
}

void FilePickerNodeSelector::refreshNavigationBreadcrumb()
{
    auto* widget = getCurrentTreeViewWidget();
    const bool show = widget && widget != mSearchWidget;

    ui->navigationBreadcrumb->setVisible(false);
    ui->lNavigationRoot->setVisible(show);
    if (show)
    {
        ui->lNavigationRoot->setText(widget->getRootText());
    }
}

QString FilePickerNodeSelector::destinationTitleText() const
{
    return QCoreApplication::translate("DestinationBreadcrumb", "Destination");
}

FilePickerNodeSelector::DestinationBannerInfo FilePickerNodeSelector::destinationBannerInfo() const
{
    return {};
}

QList<NodeSelectorBreadcrumbSegment> FilePickerNodeSelector::destinationBreadcrumbSegments() const
{
    auto* destinationWidget = selectedSearchChipTreeViewWidget();
    if (!destinationWidget)
    {
        return {};
    }

    // Selection lives in the currently visible widget (the search widget when
    // searching); destinationWidget (the selected chip) only provides the root
    // context for the path.
    auto* selectionWidget = getCurrentTreeViewWidget();
    if (!selectionWidget)
    {
        return {};
    }

    auto nodeHandle = selectionWidget->getSelectedNodeHandle();
    if (nodeHandle != mega::INVALID_HANDLE)
    {
        MegaNodeSPtr node(mMegaApi->getNodeByHandle(nodeHandle));
        if (node)
        {
            return breadcrumbSegmentsForNode(destinationWidget, node);
        }
    }

    return {};
}

QList<NodeSelectorBreadcrumbSegment> FilePickerNodeSelector::breadcrumbSegmentsForNode(
    NodeSelectorTreeViewWidget* wid,
    const std::shared_ptr<mega::MegaNode>& node) const
{
    if (!wid)
    {
        return {};
    }

    // Root label is a synthetic tab name, not a node: no handle, only fallback text.
    QList<NodeSelectorBreadcrumbSegment> segments;
    segments.append({mega::INVALID_HANDLE, wid->getRootText()});
    if (!node)
    {
        return segments;
    }

    std::unique_ptr<char[]> path(mMegaApi->getNodePath(node.get()));
    if (!path)
    {
        return segments;
    }

    const auto names = QString::fromUtf8(path.get()).split(QLatin1Char('/'), Qt::SkipEmptyParts);

    // Capture one handle per path component by walking up from the node, so the
    // breadcrumb can resolve fresh names later (e.g. when the overflow popup opens).
    QList<mega::MegaHandle> handles;
    handles.reserve(names.size());
    std::shared_ptr<mega::MegaNode> current = node;
    for (int i = 0; i < names.size() && current; ++i)
    {
        handles.prepend(current->getHandle());
        current.reset(mMegaApi->getNodeByHandle(current->getParentHandle()));
    }

    for (int i = 0; i < names.size(); ++i)
    {
        const auto handle = i < handles.size() ? handles.at(i) : mega::INVALID_HANDLE;
        segments.append({handle, names.at(i)});
    }

    return segments;
}

std::shared_ptr<mega::MegaNode>
    FilePickerNodeSelector::destinationNodeForBreadcrumb(NodeSelectorTreeViewWidget* wid) const
{
    if (!wid)
    {
        return nullptr;
    }

    if (auto selectedNode = getSelectedNode())
    {
        return selectedNode;
    }

    if (!mSelectType)
    {
        return nullptr;
    }

    const auto rootIndex = wid->getCurrentRootIndex();
    if (rootIndex.isValid())
    {
        return wid->getProxyModel()->getNode(rootIndex);
    }

    return nullptr;
}
