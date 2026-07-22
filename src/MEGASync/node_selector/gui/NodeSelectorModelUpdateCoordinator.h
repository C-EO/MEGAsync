#ifndef NODESELECTORMODELUPDATECOORDINATOR_H
#define NODESELECTORMODELUPDATECOORDINATOR_H

#include "megaapi.h"

#include <QList>
#include <QMap>
#include <QModelIndex>
#include <QMultiHash>
#include <QObject>
#include <QSet>

#include <functional>
#include <memory>

class NodeSelectorModel;
class NodeSelectorProxyModel;
class NodeSelectorTreeView;

class NodeSelectorModelUpdateCoordinator: public QObject
{
    Q_OBJECT

public:
    using GetNodeStateFn = std::function<int(const QModelIndex&, mega::MegaNode*)>;
    using GetAddedNodeParentFn = std::function<QModelIndex(mega::MegaHandle)>;

    struct Objects
    {
        NodeSelectorModel* model = nullptr;
        NodeSelectorProxyModel* proxyModel = nullptr;
        NodeSelectorTreeView* treeView = nullptr;
    };

    struct SharedState
    {
        QSet<mega::MegaHandle>& movedHandlesToSelect;
        QSet<mega::MegaHandle>& parentOfRestoredNodes;
        QMultiHash<mega::MegaHandle, mega::MegaHandle>& mergeTargetFolders;
        QSet<mega::MegaHandle>& nodesToBeReplaced;
    };

    struct Policy
    {
        GetNodeStateFn getNodeState;
        GetAddedNodeParentFn getAddedNodeParent;
    };

    NodeSelectorModelUpdateCoordinator(mega::MegaApi* megaApi,
                                       Objects objects,
                                       SharedState sharedState,
                                       Policy policy);

    bool onNodesUpdate(mega::MegaNodeList* nodes);
    void processCachedNodesUpdated();
    bool hasPendingUpdates() const;
    bool shouldUpdateImmediately(int immediateThreshold) const;

signals:
    void indexRemovedAffectingCurrentPath(const QModelIndex& index);
    void nodesRenamed(const QList<mega::MegaHandle>& handles);
    void viewStateChanged();

private:
    enum class NodeState
    {
        EXISTS,
        EXISTS_BUT_PARENT_UNINITIALISED,
        EXISTS_BUT_OUT_OF_VIEW,
        ADD,
        REMOVE,
        MOVED,
        MOVED_OUT_OF_VIEW,
        DOESNT_EXIST
    };

    struct UpdateNodesInfo
    {
        UpdateNodesInfo(mega::MegaNode* node, const QModelIndex& index):
            parentHandle(node->getParentHandle()),
            handle(node->getHandle()),
            node(std::shared_ptr<mega::MegaNode>(node->copy())),
            index(index)
        {}

        UpdateNodesInfo() = default;

        mega::MegaHandle parentHandle = mega::INVALID_HANDLE;
        mega::MegaHandle handle = mega::INVALID_HANDLE;
        std::shared_ptr<mega::MegaNode> node;
        QModelIndex index;
    };

    void updateNode(const UpdateNodesInfo& info, bool scrollTo = false);
    void removeItemByHandle(mega::MegaHandle handle);

    mega::MegaApi* mMegaApi;
    NodeSelectorModel* mModel;
    NodeSelectorProxyModel* mProxyModel;
    NodeSelectorTreeView* mTreeView;

    QSet<mega::MegaHandle>& mMovedHandlesToSelect;
    QSet<mega::MegaHandle>& mParentOfRestoredNodes;
    QMultiHash<mega::MegaHandle, mega::MegaHandle>& mMergeTargetFolders;
    QSet<mega::MegaHandle>& mNodesToBeReplaced;

    GetNodeStateFn mGetNodeState;
    GetAddedNodeParentFn mGetAddedNodeParent;

    QList<UpdateNodesInfo> mRenamedNodesByHandle;
    QList<UpdateNodesInfo> mUpdatedNodes;
    QMultiMap<mega::MegaHandle, UpdateNodesInfo> mAddedNodesByParentHandle;
    QMap<mega::MegaHandle, UpdateNodesInfo> mUpdatedNodesBeforeAdded;
    QList<UpdateNodesInfo> mRemovedNodes;
    QList<UpdateNodesInfo> mRemoveMovedNodes;
    QList<UpdateNodesInfo> mUpdatedButInvisibleNodes;
};

#endif // NODESELECTORMODELUPDATECOORDINATOR_H
