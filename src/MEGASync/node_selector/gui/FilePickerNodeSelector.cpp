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
