#include "NodeSelectorSelectionCoordinator.h"

#include "NodeSelectorMergeTargetUtils.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorModelItem.h"
#include "NodeSelectorProxyModel.h"
#include "NodeSelectorTreeView.h"

NodeSelectorSelectionCoordinator::NodeSelectorSelectionCoordinator(mega::MegaApi* megaApi,
                                                                   Objects objects,
                                                                   Policy policy):
    QObject(nullptr),
    mMegaApi(megaApi),
    mModel(objects.model),
    mProxyModel(objects.proxyModel),
    mTreeView(objects.treeView),
    mSelectIndex(std::move(policy.selectIndex)),
    mClearSelection(std::move(policy.clearSelection)),
    mOnItemDoubleClick(std::move(policy.onItemDoubleClick)),
    mSetRootIndexToTop(std::move(policy.setRootIndexToTop)),
    mWithSelectionSilenced(std::move(policy.withSelectionSilenced))
{}

void NodeSelectorSelectionCoordinator::setParentOfRestoredNodes(
    const QSet<mega::MegaHandle>& parentOfRestoredNodes)
{
    mParentOfRestoredNodes = parentOfRestoredNodes;
}

void NodeSelectorSelectionCoordinator::setMergeFolderHandles(
    const QMultiHash<mega::MegaHandle, mega::MegaHandle>& handles)
{
    mMergeTargetFolders = handles;
}

void NodeSelectorSelectionCoordinator::resetMergeFolderHandles(
    const QMultiHash<mega::MegaHandle, mega::MegaHandle>& handles)
{
    for (auto it = handles.keyValueBegin(); it != handles.keyValueEnd(); ++it)
    {
        mMergeTargetFolders.remove(it->first);
    }
}

void NodeSelectorSelectionCoordinator::setNewFolderInfo(const NewFolderInfo& info)
{
    mNewFolderInfo = info;
}

void NodeSelectorSelectionCoordinator::setSelectedNodeHandle(const mega::MegaHandle& selectedHandle)
{
    if (selectedHandle == mega::INVALID_HANDLE || mModel->rowCount() == 0)
    {
        return;
    }

    auto node = std::shared_ptr<mega::MegaNode>(mMegaApi->getNodeByHandle(selectedHandle));
    if (!node)
    {
        return;
    }

    mProxyModel->setExpandMapped(true);
    mSetRootIndexToTop();
    mModel->selectIndexesByHandleAsync(QSet<mega::MegaHandle>() << node->getHandle());
    mModel->loadTreeFromNode(node);
}

void NodeSelectorSelectionCoordinator::expandPendingIndexes()
{
    auto indexesToBeExpanded = mModel->needsToBeExpanded();
    if (indexesToBeExpanded.isEmpty())
    {
        return;
    }

    for (const auto& item: indexesToBeExpanded)
    {
        auto handle = item.first;
        QModelIndex proxyIndex;

        if (handle != mega::INVALID_HANDLE)
        {
            proxyIndex = mProxyModel->getIndexFromHandle(handle);
        }

        if (proxyIndex.isValid())
        {
            mTreeView->setExpanded(proxyIndex, true);
        }
    }
}

void NodeSelectorSelectionCoordinator::selectPendingIndexes()
{
    auto indexesToBeSelected = mModel->needsToBeSelected();
    if (indexesToBeSelected.isEmpty())
    {
        return;
    }

    mWithSelectionSilenced(
        [&]() -> bool
        {
            bool allSelected = true;
            for (const auto& item: indexesToBeSelected)
            {
                auto handle = item.first;
                if (handle != mega::INVALID_HANDLE)
                {
                    auto proxyIndex = mProxyModel->getIndexFromHandle(handle);
                    if (proxyIndex.isValid())
                    {
                        mSelectIndex(proxyIndex, true, false);
                    }
                    else
                    {
                        setSelectedNodeHandle(handle);
                        allSelected = false;
                    }
                }
            }
            return allSelected;
        });
}

void NodeSelectorSelectionCoordinator::resetMoveNodesToSelect()
{
    if (!mModel->isMovingNodes())
    {
        mMovedHandlesToSelect.clear();
    }
}

void NodeSelectorSelectionCoordinator::onItemsMoved()
{
    if (!mMovedHandlesToSelect.isEmpty() || !mMergeTargetFolders.isEmpty())
    {
        mClearSelection();
    }

    if (!mMovedHandlesToSelect.isEmpty())
    {
        mModel->selectIndexesByHandleAsync(mMovedHandlesToSelect);
    }

    if (!mMergeTargetFolders.isEmpty())
    {
        mModel->selectIndexesByHandleAsync(mMergeTargetFolders.values());
    }

    mMovedHandlesToSelect.clear();
    mParentOfRestoredNodes.clear();
    mMergeTargetFolders.clear();
}

void NodeSelectorSelectionCoordinator::onNodesAdded(
    const QList<QPointer<NodeSelectorModelItem>>& itemsAdded)
{
    if (mModel->isMovingNodes())
    {
        auto moveProcessCounter(0);

        for (const auto& item: itemsAdded)
        {
            if (!NodeSelectorMergeTargetUtils::isNodeInsideMergeTargetSubtree(
                    mMegaApi,
                    mMergeTargetFolders,
                    item->getNode().get()))
            {
                mMovedHandlesToSelect.insert(item->getNode()->getHandle());
                moveProcessCounter++;
            }
        }

        mModel->moveProcessedByNumber(moveProcessCounter);
    }
    else
    {
        for (const auto& item: itemsAdded)
        {
            checkNewFolderAdded(item);
        }
    }
}

void NodeSelectorSelectionCoordinator::checkNewFolderAdded(QPointer<NodeSelectorModelItem> item)
{
    if (!mNewFolderInfo.recentlyAdded)
    {
        return;
    }

    if (item->getNode()->getHandle() == mNewFolderInfo.handle)
    {
        auto newFolderIndex(mProxyModel->getIndexFromHandle(mNewFolderInfo.handle));
        mOnItemDoubleClick(newFolderIndex);

        mNewFolderInfo.handle = mega::INVALID_HANDLE;
        mNewFolderInfo.recentlyAdded = false;
    }
}

QSet<mega::MegaHandle>& NodeSelectorSelectionCoordinator::movedHandlesToSelect()
{
    return mMovedHandlesToSelect;
}

QSet<mega::MegaHandle>& NodeSelectorSelectionCoordinator::parentOfRestoredNodes()
{
    return mParentOfRestoredNodes;
}

QMultiHash<mega::MegaHandle, mega::MegaHandle>&
    NodeSelectorSelectionCoordinator::mergeTargetFolders()
{
    return mMergeTargetFolders;
}
