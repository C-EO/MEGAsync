#include "FilePickerNodeSelector.h"

#include "NodeSelectorModel.h"
#include "NodeSelectorTreeViewWidgetSpecializations.h"
#include "ui_NodeSelector.h"

FilePickerNodeSelector::FilePickerNodeSelector(SelectTypeSPtr selectType, QWidget* parent):
    NodeSelector(selectType, parent)
{}

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

void FilePickerNodeSelector::onItemsAboutToBeMoved(const QList<mega::MegaHandle>& handles, int)
{
    performItemsToBeMoved(handles, IncreaseOrDecrease::INCREASE, true, true);
}

void FilePickerNodeSelector::onItemsAboutToBeMovedFailed(const QList<mega::MegaHandle>& handles,
                                                         int)
{
    performItemsToBeMoved(handles, IncreaseOrDecrease::DECREASE, true, true);
}

void FilePickerNodeSelector::configureCloudDriveWidget()
{
    hideAllButNode(mCloudDriveWidget);
}

void FilePickerNodeSelector::configureIncomingSharesWidget()
{
    if (!mIncomingSharesWidget)
    {
        return;
    }

    const bool insideShare = mIncomingSharesWidget->getCurrentRootIndex().isValid();
    mIncomingSharesWidget->setColumnHidden(NodeSelectorModel::Column::USER, insideShare);
    mIncomingSharesWidget->setColumnHidden(NodeSelectorModel::Column::ACCESS, insideShare);
    mIncomingSharesWidget->setColumnHidden(NodeSelectorModel::Column::ADDED_DATE, true);
    mIncomingSharesWidget->setColumnHidden(NodeSelectorModel::Column::LAST_MODIFIED_DATE, true);

    if (insideShare)
    {
        mIncomingSharesWidget->setNonInteractiveColumns({});
    }
    else
    {
        mIncomingSharesWidget->setNonInteractiveColumns(
            {NodeSelectorModel::Column::USER, NodeSelectorModel::Column::ACCESS});
    }
}

void FilePickerNodeSelector::configureBackupsWidget()
{
    hideAllButNode(mBackupsWidget);
}

void FilePickerNodeSelector::configureRubbishWidget()
{
    hideAllButNode(mRubbishWidget);
}

void FilePickerNodeSelector::configureSearchWidget(TabType type)
{
    Q_UNUSED(type);
    hideAllButNode(mSearchWidget);
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

    ui->wTop->setVisible(false);
    ui->wSearch->setVisible(false);
}

void FilePickerNodeSelector::configureSearchTool()
{
    ui->leSearchTool->setMode(SearchLineEdit::Mode::ALWAYS_EXPANDED);

    static constexpr int SEARCH_LINE_EDIT_FIXED_WIDTH = 188;
    static constexpr int SEARCH_LINE_EDIT_FIXED_HEIGHT = 40;

    ui->leSearchTool->setFixedSize(SEARCH_LINE_EDIT_FIXED_WIDTH, SEARCH_LINE_EDIT_FIXED_HEIGHT);

    ui->headerLayout->addWidget(ui->leSearchTool);
}

void FilePickerNodeSelector::hideAllButNode(NodeSelectorTreeViewWidget* widget)
{
    if (!widget)
    {
        return;
    }
    widget->setColumnHidden(NodeSelectorModel::Column::USER, true);
    widget->setColumnHidden(NodeSelectorModel::Column::ACCESS, true);
    widget->setColumnHidden(NodeSelectorModel::Column::ADDED_DATE, true);
    widget->setColumnHidden(NodeSelectorModel::Column::LAST_MODIFIED_DATE, true);
}
