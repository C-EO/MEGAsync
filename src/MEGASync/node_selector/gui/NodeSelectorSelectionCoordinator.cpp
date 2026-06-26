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
    mOnNewFolderAdded(std::move(policy.onNewFolderAdded)),
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
    // A synchronous re-entry (an unresolved handle below triggers loadTreeFromNode, which emits
    // blockUi(false) synchronously and re-invokes this) must bail out before draining the queue,
    // so the recursion stays bounded and the pending handles survive for the real later pass.
    if (mResolvingPendingIndexes)
    {
        return;
    }

    auto indexesToBeSelected = mModel->needsToBeSelected();
    if (indexesToBeSelected.isEmpty())
    {
        return;
    }

    mResolvingPendingIndexes = true;

    // Only a restore jumps to the top root (nodes may have different parents); a move/merge
    // stays in the current folder.
    if (!mSkipTopRootOnSelect)
    {
        mSetRootIndexToTop();
    }
    mSkipTopRootOnSelect = false;

    mWithSelectionSilenced(
        [&]() -> bool
        {
            mClearSelection();
            bool allSelected = true;
            for (const auto& item: indexesToBeSelected)
            {
                auto handle = item.first;
                if (handle != mega::INVALID_HANDLE)
                {
                    auto proxyIndex = mProxyModel->getIndexFromHandle(handle);
                    if (proxyIndex.isValid())
                    {
                        mHandlesPendingLoad.remove(handle);
                        mSelectIndex(proxyIndex, true, false);
                    }
                    else if (!mHandlesPendingLoad.contains(handle))
                    {
                        // First time this handle cannot be mapped: load its tree path once so it
                        // becomes selectable. The load is triggered only once; re-triggering it on
                        // every pass is what caused the infinite recursion.
                        mHandlesPendingLoad.insert(handle);
                        setSelectedNodeHandle(handle);
                        allSelected = false;
                    }
                    else
                    {
                        // The load is already in flight: an ancestor's children are still being
                        // fetched asynchronously (the worker delivers them through a queued
                        // nodesReady, so the rows are inserted only after this synchronous pass
                        // returns). Re-queue the handle WITHOUT re-triggering the load, so the next
                        // pass -- fired once the rows are actually inserted -- resolves and selects
                        // it. Dropping it here is what left incoming-share files unselected.
                        mModel->selectIndexesByHandleAsync(QSet<mega::MegaHandle>() << handle);
                        allSelected = false;
                    }
                }
            }
            return allSelected;
        });

    mResolvingPendingIndexes = false;
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
    // A move/merge stays in the current folder; only a restore jumps to the top root.
    mSkipTopRootOnSelect = mParentOfRestoredNodes.isEmpty();

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

    // Keep mMovedHandlesToSelect populated: the moved node's model item is recreated
    // (remove + re-insert) while the move settles, which drops the selection of its row.
    // reapplyMovedSelection() re-selects it by handle whenever its row reappears.
    mParentOfRestoredNodes.clear();
    mMergeTargetFolders.clear();
}

void NodeSelectorSelectionCoordinator::reapplyMovedSelection()
{
    if (mMovedHandlesToSelect.isEmpty())
    {
        return;
    }

    mWithSelectionSilenced(
        [&]() -> bool
        {
            // Clear first so only the moved nodes remain selected: the restored selection
            // model keeps the pre-move selection (e.g. the destination folder the user had
            // open), which must not stay selected after the move.
            mClearSelection();
            for (const auto& handle: std::as_const(mMovedHandlesToSelect))
            {
                auto proxyIndex = mProxyModel->getIndexFromHandle(handle);
                if (proxyIndex.isValid())
                {
                    mSelectIndex(proxyIndex, true, false);
                }
            }
            return false;
        });
}

void NodeSelectorSelectionCoordinator::clearMovedSelection()
{
    mMovedHandlesToSelect.clear();
}

void NodeSelectorSelectionCoordinator::onNodesAdded(
    const QList<QPointer<NodeSelectorModelItem>>& itemsAdded)
{
    if (mModel->isMovingNodes())
    {
        auto moveProcessCounter(0);

        for (const auto& item: itemsAdded)
        {
            if (!item)
            {
                continue;
            }

            auto node = item->getNode();
            if (!node)
            {
                continue;
            }

            if (!NodeSelectorMergeTargetUtils::isNodeInsideMergeTargetSubtree(mMegaApi,
                                                                              mMergeTargetFolders,
                                                                              node.get()))
            {
                mMovedHandlesToSelect.insert(node->getHandle());
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

    if (!item || !item->getNode())
    {
        return;
    }

    if (item->getNode()->getHandle() == mNewFolderInfo.handle)
    {
        auto newFolderIndex(mProxyModel->getIndexFromHandle(mNewFolderInfo.handle));
        mOnNewFolderAdded(newFolderIndex);

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
