#include "FilePickerNodeSelector.h"

#include "megaapi.h"
#include "NodeSelectorDestinationBreadcrumb.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorProxyModel.h"
#include "NodeSelectorTreeViewWidgetSpecializations.h"
#include "ui_NodeSelector.h"

#include <QCoreApplication>

FilePickerNodeSelector::FilePickerNodeSelector(SelectTypeSPtr selectType, QWidget* parent):
    NodeSelector(selectType, parent)
{
    connect(ui->destinationBreadcrumb,
            &NodeSelectorDestinationBreadcrumb::clearRequested,
            this,
            &FilePickerNodeSelector::onBreadcrumbClearRequested);
}

NodeSelector::ClearTypes FilePickerNodeSelector::searchClearType() const
{
    return (ClearType::CLEAR_ON_CLEAR_SEARCH_LINE_EDIT | ClearType::CLEAR_ON_TAB_CHANGE);
}

void FilePickerNodeSelector::handleSearch(const QString& text)
{
    if (!mSearchWidget)
    {
        return;
    }

    auto sourceTab = currentSearchSourceTab();
    if (!sourceTab.has_value())
    {
        return;
    }

    const bool sameAsActiveSearch = isCurrentTabSearchActive() && text == mLastSearchText;

    if (!sameAsActiveSearch)
    {
        mSearchSourceTab = sourceTab;
        mLastSearchText = text;
        mSearchWidget->setSearchScope(tabTypeForItem(sourceTab.value()));
        mSearchWidget->search(text);
    }

    ui->stackedWidget->setCurrentWidget(mSearchWidget);
}

void FilePickerNodeSelector::handleSearchHidden()
{
    clearCurrentTabSearch(true);
}

void FilePickerNodeSelector::clearSearch()
{
    ui->leSearchTool->onClearClicked();
    mLastSearchText.clear();
    mSearchSourceTab.reset();

    if (mSearchWidget)
    {
        mSearchWidget->resetSearchState();
    }
}

void FilePickerNodeSelector::refreshDestinationBreadcrumb()
{
    const bool shouldShowPath =
        mSelectType && mSelectType->showsDestinationBreadcrumb() && getSearchAwareTargetWidget();
    const auto banner = destinationBannerInfo();

    const bool shouldShowBanner = !banner.text.isEmpty() && banner.type != BannerWidget::Type::NONE;

    // Mutually exclusive: banner wins over breadcrumb when both would apply.
    if (shouldShowBanner)
    {
        ui->destinationBanner->setType(banner.type);
        ui->destinationBanner->setTitle(banner.text);
        ui->destinationBanner->setVisible(true);
        ui->destinationBreadcrumb->setVisible(false);
        ui->destinationBreadcrumb->setPathSegments({});
        return;
    }

    ui->destinationBanner->setVisible(false);

    if (!shouldShowPath)
    {
        ui->destinationBreadcrumb->setVisible(false);
        ui->destinationBreadcrumb->setPathSegments({});
        return;
    }

    ui->destinationBreadcrumb->setVisible(true);
    auto path{destinationBreadcrumbSegments()};
    if (path.isEmpty())
    {
        ui->destinationBreadcrumb->setPathSegments(QStringList()
                                                   << destinationBreadcrumbEmptyText());
    }
    else
    {
        ui->destinationBreadcrumb->setPathSegments(path);
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

void FilePickerNodeSelector::configureSearchWidget(TabType type)
{
    if (!mSearchWidget)
    {
        return;
    }

    if (type == TabType::INCOMING_SHARE)
    {
        configureIncomingSharesTableColumns(mSearchWidget);
    }
    else
    {
        configureTableColumns(mSearchWidget);
    }
}

void FilePickerNodeSelector::configureSidebar()
{
    static constexpr int COLLAPSED_SIDEBAR_WIDTH = 64;
    static constexpr int COLLAPSED_TAB_HEIGHT = 36;

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
        tab->setMinimumHeight(COLLAPSED_TAB_HEIGHT);
        tab->setMaximumHeight(COLLAPSED_TAB_HEIGHT);
        tab->style()->unpolish(tab);
        tab->style()->polish(tab);
    };

    collapseTab(ui->fCloudDrive);
    collapseTab(ui->fIncomingShares);
    collapseTab(ui->fBackups);
    collapseTab(ui->fRubbish);
    collapseTab(ui->fSearch);

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

    ui->wSearch->setVisible(false);
}

void FilePickerNodeSelector::configureHeader()
{
    ui->leSearchTool->setMode(SearchLineEdit::Mode::ALWAYS_EXPANDED);

    static constexpr int SEARCH_LINE_EDIT_FIXED_WIDTH = 188;
    static constexpr int SEARCH_LINE_EDIT_FIXED_HEIGHT = 40;

    ui->leSearchTool->setFixedSize(SEARCH_LINE_EDIT_FIXED_WIDTH, SEARCH_LINE_EDIT_FIXED_HEIGHT);
    ui->actionButtonsContainer->setVisible(false);
    ui->navigationBreadcrumb->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    if (auto* topRowLayout = ui->headerLayout)
    {
        ui->headerContainerLayout->removeWidget(ui->navigationBreadcrumb);
        topRowLayout->insertWidget(0, ui->navigationBreadcrumb, 0, Qt::AlignLeft | Qt::AlignTop);
    }
}

QString FilePickerNodeSelector::destinationTitleText() const
{
    return QCoreApplication::translate("NodeSelectorDestinationBreadcrumb", "Destination");
}

FilePickerNodeSelector::DestinationBannerInfo FilePickerNodeSelector::destinationBannerInfo() const
{
    return {};
}

QStringList FilePickerNodeSelector::destinationBreadcrumbSegments() const
{
    auto* destinationWidget = getSearchAwareTargetWidget();
    if (!destinationWidget)
    {
        return {};
    }

    // Selection lives in the currently visible widget (the search widget when
    // searching); destinationWidget only provides the root context for the path.
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

QStringList FilePickerNodeSelector::breadcrumbSegmentsForNode(
    NodeSelectorTreeViewWidget* wid,
    const std::shared_ptr<mega::MegaNode>& node) const
{
    if (!wid)
    {
        return {};
    }

    QStringList segments{wid->getRootText()};
    if (!node)
    {
        return segments;
    }

    std::unique_ptr<char[]> path(mMegaApi->getNodePath(node.get()));
    if (!path)
    {
        return segments;
    }

    const auto nodeSegments =
        QString::fromUtf8(path.get()).split(QLatin1Char('/'), Qt::SkipEmptyParts);
    segments.append(nodeSegments);
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
