#ifndef NODESELECTORTREEVIEWWIDGET_H
#define NODESELECTORTREEVIEWWIDGET_H

#include "ButtonIconManager.h"
#include "megaapi.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorModelUpdateCoordinator.h"
#include "NodeSelectorNavigation.h"
#include "NodeSelectorNodeActions.h"
#include "NodeSelectorSelectionCoordinator.h"
#include "NodeSelectorSelectTypes.h"
#include "NodeSelectorTabTypes.h"
#include "QTMegaListener.h"

#include <QDebug>
#include <QItemSelectionModel>
#include <QMap>
#include <QPersistentModelIndex>
#include <QTimer>
#include <QWidget>

#include <memory>

class NodeSelectorProxyModel;
class NodeSelectorModel;
class NodeSelectorModelItem;
class NodeSelectorTreeView;

struct MessageInfo;

namespace Ui
{
class NodeSelectorTreeViewWidget;
}

class NodeSelectorTreeViewWidget: public QWidget
{
    Q_OBJECT

public:
    static const int LOADING_VIEW_THRESSHOLD;
    static const int LABEL_ELIDE_MARGIN;
    static const char* FULL_NAME_PROPERTY;

    explicit NodeSelectorTreeViewWidget(SelectTypeSPtr mode, QWidget* parent = nullptr);
    ~NodeSelectorTreeViewWidget();

    void init();

    mega::MegaHandle getSelectedNodeHandle();
    QList<mega::MegaHandle> getMultiSelectionNodeHandle();
    QModelIndexList getSelectedIndexes() const;
    bool containsTakenDownSelected() const;
    void navigateToItem(const mega::MegaHandle& handle);
    void setSelectedNodeHandle(const mega::MegaHandle& selectedHandle);

    template<class Container>
    void setAsyncSelectedNodeHandle(const Container& selectedHandles)
    {
        clearSelection();
        mModel->selectIndexesByHandleAsync<Container>(selectedHandles);
    }

    void selectPendingIndexes();

    virtual void treeViewWidgetSelected() {}

    void clearSelection();
    bool isSelectionCorrect();

    void abort();
    NodeSelectorModelItem* rootItem();
    QModelIndex getCurrentRootIndex();
    NodeSelectorProxyModel* getProxyModel();
    bool isInRootView() const;
    bool isEmpty() const;

    bool onNodesUpdate(mega::MegaApi*, mega::MegaNodeList* nodes);

    void enableDragAndDrop(bool enable);

    bool increaseMovingNodes(int number);
    bool decreaseMovingNodes(int number);
    bool areItemsAboutToBeMovedFromHere(mega::MegaHandle firstHandleMoved);

    mega::MegaHandle getHandleByIndex(const QModelIndex& idx);

    void addHandleToBeReplaced(mega::MegaHandle handle);
    void setParentOfRestoredNodes(const QSet<mega::MegaHandle>& parentOfRestoredNodes);

    using TargetHandle = mega::MegaHandle;
    using SourceHandle = mega::MegaHandle;
    void setMergeFolderHandles(const QMultiHash<SourceHandle, TargetHandle>& handles);
    void resetMergeFolderHandles(const QMultiHash<SourceHandle, TargetHandle>& handles);

    bool isUiBlocked();
    bool canGoBack() const;
    bool canGoForward() const;
    bool shouldShowNavigationButtons() const;

    void dropIntoRootIndex(QDropEvent* event);
    void goBack();
    void goForward();

    using NewFolderInfo = NodeSelectorSelectionCoordinator::NewFolderInfo;

    void setNewFolderInfo(const NewFolderInfo& newNewFolderInfo);

public slots:
    virtual void checkViewOnModelChange();
    void setLoadingSceneVisible(bool visible);
    void notifyViewStateChanged();
    void notifyButtonsStateChanged();

signals:
    void enterKeyPressed();
    void onCustomButtonClicked(uint id);
    void newFolderRequested();
    void viewReady();
    void uiIsBlocked(bool state);
    void selectionIsCorrect(bool state);
    void viewStateChanged();
    void viewButtonsStateChanged();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    bool event(QEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void selectionChanged(const QModelIndexList& selected);
    QModelIndex getParentIncomingShareByIndex(QModelIndex idx);

    SelectTypeSPtr getSelectType()
    {
        return mSelectType;
    }

    virtual void setViewPage();

    virtual bool showEmptyView()
    {
        return true;
    }

    virtual void makeCustomConnections() {}

    virtual bool isNodeCompatibleWithModel(mega::MegaNode*)
    {
        return false;
    }

    virtual void onRootIndexChanged(const QModelIndex& source_idx);
    virtual QModelIndex getAddedNodeParent(mega::MegaHandle parentHandle);
    QModelIndex getRootIndexFromIndex(const QModelIndex& index);
    void selectIndex(const QModelIndex& index, bool setCurrent, bool exclusiveSelect = false);
    void selectIndex(const mega::MegaHandle& handle, bool setCurrent, bool exclusiveSelect = false);

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

    virtual NodeState getNodeOnModelState(const QModelIndex& index, mega::MegaNode* node);

    struct EmptyLabelInfo
    {
        QString title;
        QString description;
    };

    virtual EmptyLabelInfo getEmptyLabel();

    Ui::NodeSelectorTreeViewWidget* ui;
    std::shared_ptr<NodeSelectorProxyModel> mProxyModel;
    std::unique_ptr<NodeSelectorModel> mModel;
    std::unique_ptr<NodeSelectorSelectionCoordinator> mSelectionCoordinator;
    std::unique_ptr<NodeSelectorModelUpdateCoordinator> mModelUpdateCoordinator;
    NodeSelectorNavigation mNavigation;
    NodeSelectorNodeActions mNodeActions;
    mega::MegaApi* mMegaApi;
    SelectTypeSPtr mSelectType;

protected slots:
    // Title
    void updateRootTitle();

    // Invoked after the proxy finishes processing a level load
    virtual void onLevelLoaded();

private slots:
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void onModelModified();
    void onDeleteClicked(const QList<mega::MegaHandle>& handles,
                         bool permanently,
                         bool showConfirmationMessageBox);
    void onLeaveShareClicked(const QList<mega::MegaHandle>& handles);
    void onRenameClicked();
    void onGenMEGALinkClicked(const QList<mega::MegaHandle>& handles);
    virtual void onItemDoubleClick(const QModelIndex& index);
    void onRemoveIndexFromGoBack(const QModelIndex& index);
    void onSectionResized();
    void onUiBlocked(bool state);
    void processCachedNodesUpdated();

private:
    bool mManuallyResizedColumn;
    int mResizeEventsReceived;
    QTimer mResizeEventsTimer;

    virtual bool isAllowedToEnterInIndex(const QModelIndex& idx);
    void setRootIndex(const QModelIndex& proxy_idx);
    virtual QIcon getEmptyIcon();
    void setEmptyFolderPage();

    QModelIndex getIndexFromHandle(const mega::MegaHandle& handle);
    virtual QString getRootText() = 0;
    virtual std::shared_ptr<NodeSelectorProxyModel> createProxyModel();
    virtual std::unique_ptr<NodeSelectorModel> createModel() = 0;

    virtual bool isCurrentRootIndexReadOnly()
    {
        return false;
    }

    virtual bool isSelectionReadOnly(const QModelIndexList&)
    {
        return false;
    }

    virtual bool isCurrentSelectionReadOnly()
    {
        return false;
    }

    void selectionHasChanged(const QModelIndexList& selected);

    void checkOkButton(const QModelIndexList& selected);

    // Empty messages
    void initEmptyMessages();

    // Column width
    QList<int> mVisibleColumns;
    void updateColumnsWidth(bool updateVisibleColumnCounter);

    ButtonIconManager mButtonIconManager;
    bool first;
    bool mUiBlocked;
    bool mWasEmpty;
    bool mNewFolderButtonVisible = true;

    // Containers used to ignore specific nodes updates
    QSet<mega::MegaHandle> mNodesToBeReplaced;

    QTimer mNodesUpdateTimer;

    friend class DownloadType;
    friend class SyncType;
    friend class UploadType;
    friend class StreamType;
    friend class CloudDriveType;
    friend class SelectType;
};

#endif // NODESELECTORTREEVIEWWIDGET_H
