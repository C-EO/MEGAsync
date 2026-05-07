#include "NodeSelectorNavigation.h"

#include <QMap>

#include <memory>

NodeSelectorNavigation::NodeSelectorNavigation(mega::MegaApi* megaApi):
    mMegaApi(megaApi)
{}

bool NodeSelectorNavigation::canGoBack() const
{
    return !mBackwardHandles.isEmpty();
}

bool NodeSelectorNavigation::canGoForward() const
{
    return !mForwardHandles.isEmpty();
}

bool NodeSelectorNavigation::shouldShowNavigationButtons() const
{
    return canGoBack() || canGoForward();
}

std::optional<mega::MegaHandle> NodeSelectorNavigation::goBack(mega::MegaHandle currentRootHandle)
{
    if (!canGoBack())
    {
        return std::nullopt;
    }

    if (currentRootHandle != mega::INVALID_HANDLE)
    {
        appendToForward(currentRootHandle);
    }

    const auto targetHandle = mBackwardHandles.takeLast();
    return targetHandle;
}

std::optional<mega::MegaHandle>
    NodeSelectorNavigation::goForward(mega::MegaHandle currentRootHandle)
{
    if (!canGoForward())
    {
        return std::nullopt;
    }

    appendToBackward(currentRootHandle);

    const auto targetHandle = mForwardHandles.takeLast();
    return targetHandle;
}

void NodeSelectorNavigation::onNavigateInto(mega::MegaHandle currentRootHandle,
                                            mega::MegaHandle targetHandle)
{
    appendToBackward(currentRootHandle);
    removeFromForward(targetHandle);
}

void NodeSelectorNavigation::onRootChanged(mega::MegaHandle rootHandle)
{
    const auto handlePos = mBackwardHandles.indexOf(rootHandle);
    if (handlePos >= 0)
    {
        auto it = mBackwardHandles.begin();
        std::advance(it, handlePos);
        mBackwardHandles.erase(it, mBackwardHandles.end());
    }
}

void NodeSelectorNavigation::onHandleRemoved(mega::MegaHandle handle)
{
    if (mForwardHandles.contains(handle))
    {
        mForwardHandles.removeLast();
    }

    remove(handle);
}

bool NodeSelectorNavigation::hasBackwardHandle(mega::MegaHandle handle) const
{
    return mBackwardHandles.contains(handle);
}

void NodeSelectorNavigation::clearBackward()
{
    mBackwardHandles.clear();
}

void NodeSelectorNavigation::clear()
{
    mBackwardHandles.clear();
    mForwardHandles.clear();
}

void NodeSelectorNavigation::removeFromForward(const mega::MegaHandle handle)
{
    if (mForwardHandles.isEmpty())
    {
        return;
    }

    auto pNode = std::unique_ptr<mega::MegaNode>(mMegaApi->getNodeByHandle(handle));

    QMap<mega::MegaHandle, mega::MegaHandle> parentHandles;
    while (pNode)
    {
        const auto actualHandle = pNode->getHandle();
        pNode.reset(mMegaApi->getParentNode(pNode.get()));

        auto parentHandle = mega::INVALID_HANDLE;
        if (pNode)
        {
            parentHandle = pNode->getHandle();
        }

        parentHandles.insert(parentHandle, actualHandle);
    }

    pNode.reset(mMegaApi->getNodeByHandle(mForwardHandles.last()));

    QMap<mega::MegaHandle, mega::MegaHandle> actualListParentHandles;
    while (pNode)
    {
        const auto actualHandle = pNode->getHandle();
        pNode.reset(mMegaApi->getParentNode(pNode.get()));

        auto parentHandle = mega::INVALID_HANDLE;
        if (pNode)
        {
            parentHandle = pNode->getHandle();
        }

        actualListParentHandles.insert(parentHandle, actualHandle);
    }

    for (auto it = actualListParentHandles.begin(); it != actualListParentHandles.end(); ++it)
    {
        if (parentHandles.contains(it.key()))
        {
            mForwardHandles.clear();
            return;
        }
    }
}

void NodeSelectorNavigation::remove(const mega::MegaHandle handle)
{
    mBackwardHandles.removeAll(handle);

    const auto forwardPos = static_cast<int>(mForwardHandles.indexOf(handle));
    for (int i = 0; i <= forwardPos; ++i)
    {
        mForwardHandles.removeFirst();
    }
}

void NodeSelectorNavigation::appendToBackward(const mega::MegaHandle handle)
{
    if (!mBackwardHandles.contains(handle))
    {
        mBackwardHandles.append(handle);
    }
}

void NodeSelectorNavigation::appendToForward(const mega::MegaHandle handle)
{
    if (!mForwardHandles.contains(handle))
    {
        mForwardHandles.append(handle);
    }
}
