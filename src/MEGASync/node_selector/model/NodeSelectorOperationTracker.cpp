#include "NodeSelectorOperationTracker.h"

#include <QMutexLocker>

#include <algorithm>

bool NodeSelectorOperationTracker::beginMoveOperation(int number)
{
    if (number <= 0)
    {
        return false;
    }

    QMutexLocker lock(&mMutex);
    const auto wasIdle = mPendingMoveOperations.isEmpty();
    mPendingMoveOperations.enqueue(number);
    return wasIdle;
}

bool NodeSelectorOperationTracker::consumeMoveOperations(int number)
{
    if (number <= 0)
    {
        return false;
    }

    QMutexLocker lock(&mMutex);
    if (mPendingMoveOperations.isEmpty())
    {
        return false;
    }

    auto remainingToConsume = number;

    while (remainingToConsume > 0 && !mPendingMoveOperations.isEmpty())
    {
        auto& currentOperation = mPendingMoveOperations.head();
        const auto consumedItems = std::min(currentOperation, remainingToConsume);

        currentOperation -= consumedItems;
        remainingToConsume -= consumedItems;

        if (currentOperation == 0)
        {
            mPendingMoveOperations.dequeue();
        }
    }

    return true;
}

void NodeSelectorOperationTracker::clearMoveOperations()
{
    QMutexLocker lock(&mMutex);
    mPendingMoveOperations.clear();
}

bool NodeSelectorOperationTracker::hasMoveOperations() const
{
    QMutexLocker lock(&mMutex);
    return !mPendingMoveOperations.isEmpty();
}

int NodeSelectorOperationTracker::pendingMoveItems() const
{
    QMutexLocker lock(&mMutex);
    auto totalPendingItems = 0;

    for (const auto& pendingOperation: mPendingMoveOperations)
    {
        totalPendingItems += pendingOperation;
    }

    return totalPendingItems;
}

void NodeSelectorOperationTracker::beginRequestGroup(int type,
                                                     const QList<mega::MegaHandle>& handles)
{
    if (handles.isEmpty())
    {
        return;
    }

    QMutexLocker lock(&mMutex);

    RequestGroup requestGroup;
    requestGroup.type = type;
    requestGroup.pendingHandles = handles;
    mRequestGroups.append(requestGroup);
}

NodeSelectorOperationTracker::FinishedRequestGroup
    NodeSelectorOperationTracker::finishRequest(mega::MegaHandle handle,
                                                bool failed,
                                                int movedItemCategory)
{
    QMutexLocker lock(&mMutex);
    FinishedRequestGroup result;

    if (mRequestGroups.isEmpty())
    {
        return result;
    }

    auto groupIt = std::find_if(mRequestGroups.begin(),
                                mRequestGroups.end(),
                                [handle](const RequestGroup& group)
                                {
                                    return handle == mega::INVALID_HANDLE ||
                                           group.pendingHandles.contains(handle);
                                });

    if (groupIt == mRequestGroups.end())
    {
        groupIt = mRequestGroups.begin();
    }

    result.matched = true;
    result.type = groupIt->type;

    if (!groupIt->pendingHandles.isEmpty())
    {
        if (handle == mega::INVALID_HANDLE)
        {
            groupIt->pendingHandles.removeFirst();
        }
        else
        {
            groupIt->pendingHandles.removeOne(handle);
        }
    }

    if (failed)
    {
        groupIt->failedHandles.append(handle);
    }

    groupIt->movedItemCategories |= movedItemCategory;

    if (groupIt->pendingHandles.isEmpty())
    {
        result.groupFinished = true;
        result.failedHandles = groupIt->failedHandles;
        result.movedItemCategories = groupIt->movedItemCategories;
        mRequestGroups.erase(groupIt);
    }

    return result;
}

bool NodeSelectorOperationTracker::hasRequestGroups() const
{
    QMutexLocker lock(&mMutex);
    return !mRequestGroups.isEmpty();
}
