#ifndef NODESELECTORMERGETARGETUTILS_H
#define NODESELECTORMERGETARGETUTILS_H

#include "megaapi.h"

#include <QMultiHash>

#include <memory>

namespace NodeSelectorMergeTargetUtils
{
inline bool
    isMergeTargetHandle(const QMultiHash<mega::MegaHandle, mega::MegaHandle>& mergeTargetFolders,
                        mega::MegaHandle handle)
{
    if (handle == mega::INVALID_HANDLE)
    {
        return false;
    }

    for (auto it = mergeTargetFolders.constKeyValueBegin();
         it != mergeTargetFolders.constKeyValueEnd();
         ++it)
    {
        if (it->second == handle)
        {
            return true;
        }
    }

    return false;
}

inline bool isNodeInsideMergeTargetSubtree(
    mega::MegaApi* megaApi,
    const QMultiHash<mega::MegaHandle, mega::MegaHandle>& mergeTargetFolders,
    mega::MegaNode* node)
{
    if (!megaApi || !node || mergeTargetFolders.isEmpty())
    {
        return false;
    }

    auto ancestorHandle = node->getParentHandle();

    while (ancestorHandle != mega::INVALID_HANDLE)
    {
        if (isMergeTargetHandle(mergeTargetFolders, ancestorHandle))
        {
            return true;
        }

        std::unique_ptr<mega::MegaNode> parentNode(megaApi->getNodeByHandle(ancestorHandle));
        if (!parentNode)
        {
            break;
        }

        ancestorHandle = parentNode->getParentHandle();
    }

    return false;
}
}

#endif // NODESELECTORMERGETARGETUTILS_H
