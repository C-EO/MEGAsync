#ifndef NODESELECTORSELECTIONCOORDINATOR_H
#define NODESELECTORSELECTIONCOORDINATOR_H

#include "megaapi.h"

#include <QList>
#include <QModelIndex>
#include <QMultiHash>
#include <QObject>
#include <QPointer>
#include <QSet>

#include <functional>

class NodeSelectorModel;
class NodeSelectorModelItem;
class NodeSelectorProxyModel;
class NodeSelectorTreeView;

class NodeSelectorSelectionCoordinator: public QObject
{
    Q_OBJECT

public:
    struct NewFolderInfo
    {
        mega::MegaHandle handle = mega::INVALID_HANDLE;
        bool recentlyAdded = false;
    };

    struct Objects
    {
        NodeSelectorModel* model = nullptr;
        NodeSelectorProxyModel* proxyModel = nullptr;
        NodeSelectorTreeView* treeView = nullptr;
    };

    struct Policy
    {
        std::function<void(const QModelIndex&, bool, bool)> selectIndex;
        std::function<void()> clearSelection;
        std::function<void(const QModelIndex&)> onItemDoubleClick;
        std::function<void()> setRootIndexToTop;
        std::function<void(const QModelIndexList&)> selectionHasChanged;
        std::function<void(std::function<void()>)> withSelectionSilenced;
        std::function<void()> onSelectionChanged;
    };

    NodeSelectorSelectionCoordinator(mega::MegaApi* megaApi, Objects objects, Policy policy);

    void setParentOfRestoredNodes(const QSet<mega::MegaHandle>& parentOfRestoredNodes);
    void setMergeFolderHandles(const QMultiHash<mega::MegaHandle, mega::MegaHandle>& handles);
    void resetMergeFolderHandles(const QMultiHash<mega::MegaHandle, mega::MegaHandle>& handles);
    void setNewFolderInfo(const NewFolderInfo& info);

    void setSelectedNodeHandle(const mega::MegaHandle& selectedHandle);
    void expandPendingIndexes();
    void selectPendingIndexes();
    void resetMoveNodesToSelect();

    QSet<mega::MegaHandle>& movedHandlesToSelect();
    QSet<mega::MegaHandle>& parentOfRestoredNodes();
    QMultiHash<mega::MegaHandle, mega::MegaHandle>& mergeTargetFolders();

public slots:
    void onItemsMoved();
    void onNodesAdded(const QList<QPointer<NodeSelectorModelItem>>& itemsAdded);

private:
    void checkNewFolderAdded(QPointer<NodeSelectorModelItem> item);

    mega::MegaApi* mMegaApi;
    NodeSelectorModel* mModel;
    NodeSelectorProxyModel* mProxyModel;
    NodeSelectorTreeView* mTreeView;

    std::function<void(const QModelIndex&, bool, bool)> mSelectIndex;
    std::function<void()> mClearSelection;
    std::function<void(const QModelIndex&)> mOnItemDoubleClick;
    std::function<void()> mSetRootIndexToTop;
    std::function<void(const QModelIndexList&)> mSelectionHasChanged;
    std::function<void(std::function<void()>)> mWithSelectionSilenced;
    std::function<void()> mOnSelectionChanged;

    QSet<mega::MegaHandle> mMovedHandlesToSelect;
    QSet<mega::MegaHandle> mParentOfRestoredNodes;
    QMultiHash<mega::MegaHandle, mega::MegaHandle> mMergeTargetFolders;
    NewFolderInfo mNewFolderInfo;
};

#endif // NODESELECTORSELECTIONCOORDINATOR_H
