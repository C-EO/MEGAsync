#ifndef NODESELECTORTREEVIEWWIDGET_H
#define NODESELECTORTREEVIEWWIDGET_H

#include "ButtonIconManager.h"
#include "IncomingShareHeaderWidget.h"
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
#include <QSet>
#include <QTimer>
#include <QWidget>

#include <memory>
#include <optional>

class NodeSelectorProxyModel;
class NodeSelectorModel;
class NodeSelectorModelItem;
class NodeSelectorTreeView;
class NodeSelectorDelegate;

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

    enum TabItem
    {
        CLOUD_DRIVE = 0,
        SHARES,
        BACKUPS,
        RUBBISH,
        SEARCH
    };
    Q_ENUM(TabItem)

    explicit NodeSelectorTreeViewWidget(SelectTypeSPtr mode,
                                        TabItem tabType,
                                        QWidget* parent = nullptr);
    ~NodeSelectorTreeViewWidget();

    void init();

    QList<mega::MegaHandle> getMultiSelectionNodeHandle();
    QModelIndexList getSelectedIndexes() const;
    bool containsTakenDownSelected() const;
    mega::MegaHandle getSelectedNodeHandle() const;
    void navigateToItem(const mega::MegaHandle& handle);
    void setSelectedNodeHandle(const mega::MegaHandle& selectedHandle);

    template<class Container>
    void setAsyncSelectedNodeHandle(const Container& selectedHandles)
    {
        clearSelection();
        mModel->selectIndexesByHandleAsync<Container>(selectedHandles);
    }

    void selectPendingIndexes();

    bool clearSelection();

    void abort();
    void moveToTopRootIndex();
    NodeSelectorModelItem* rootItem();
    QModelIndex getCurrentRootIndex() const;
    NodeSelectorProxyModel* getProxyModel();
    bool isInRootView() const;
    bool isEmpty() const;

    bool onNodesUpdate(mega::MegaApi*, mega::MegaNodeList* nodes);

    void enableDragAndDrop(bool enable);

    bool increaseMovingNodes(int number);
    bool decreaseMovingNodes(int number);
    void finishMovingNodes();
    bool areItemsAboutToBeMovedFromHere(mega::MegaHandle firstHandleMoved);

    mega::MegaHandle getHandleByIndex(const QModelIndex& idx) const;

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

    virtual std::optional<IncomingShareHeaderData> incomingShareHeaderData() const
    {
        return std::nullopt;
    }

    virtual QString getRootText() = 0;

    void setColumnHidden(int column, bool hidden);
    void setNonInteractiveColumns(const QSet<int>& columns);

    TabItem getTabType() const
    {
        return mTabType;
    }

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
    void selectionHasChanged();
    void viewStateChanged();
    void viewButtonsStateChanged();
    void modelModified();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    bool event(QEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void selectionChanged(const QModelIndexList& selected);
    QModelIndex getParentIncomingShareByIndex(QModelIndex idx) const;

    SelectTypeSPtr getSelectType()
    {
        return mSelectType;
    }

    virtual void setViewPage();

    virtual bool showEmptyView()
    {
        return true;
    }

    virtual void makeViewConnections() {}

    virtual bool isNodeCompatibleWithModel(mega::MegaNode*)
    {
        return false;
    }

    void onRootIndexChanged(const QModelIndex& source_idx);
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

    enum class EmptyStateKind
    {
        NONE,
        ROOT,
        FOLDER
    };

    virtual NodeState getNodeOnModelState(const QModelIndex& index, mega::MegaNode* node);

    virtual SelectType::EmptyPageInfo getEmptyRootPageInfo();
    void showRootEmptyState();

    virtual NodeSelectorDelegate* createItemDelegate(QObject* parent);

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
    virtual bool isDownloadAllowed() const;
    void setRootIndex(const QModelIndex& proxy_idx);
    void setEmptyFolderPage();
    void showFolderEmptyState();
    void applyEmptyState(const SelectType::EmptyPageInfo& info, EmptyStateKind kind);
    void setEmptyStateButtonsVisibility(const SelectType::EmptyPageInfo& info);
    void updateEmptyStateButtonsVisibility();
    QModelIndex currentFolderIndex() const;

    QModelIndex getIndexFromHandle(const mega::MegaHandle& handle);
    virtual std::shared_ptr<NodeSelectorProxyModel> createProxyModel();
    virtual std::unique_ptr<NodeSelectorModel> createModel() = 0;

    virtual bool isCurrentRootIndexReadOnly() const
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

    void onSelectionHasChanged();

    void checkOkButton(const QModelIndexList& selected);

    // Empty messages
    void initEmptyRootPageMessages();
    void initEmptyFolderMessages();

    // Column width
    QList<int> mVisibleColumns;
    void updateColumnsWidth(bool updateVisibleColumnCounter);

    ButtonIconManager mButtonIconManager;
    bool first;
    bool mUiBlocked;
    bool mWasEmpty;
    bool mNewFolderButtonVisible = true;
    EmptyStateKind mCurrentEmptyStateKind = EmptyStateKind::NONE;

    // Containers used to ignore specific nodes updates
    QSet<mega::MegaHandle> mNodesToBeReplaced;

    QTimer mNodesUpdateTimer;

    TabItem mTabType;

    friend class DownloadType;
    friend class SyncType;
    friend class UploadType;
    friend class StreamType;
    friend class CloudDriveType;
    friend class SelectType;
};

#endif // NODESELECTORTREEVIEWWIDGET_H
