#ifndef NODESELECTOR_H
#define NODESELECTOR_H

#include "ButtonIconManager.h"
#include "NodeSelectorTreeViewWidget.h"

#include <QDialog>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QItemSelection>
#include <QMap>
#include <QPixmap>
#include <QPointer>
#include <QStringList>
#include <QTimer>

#include <memory>
#include <optional>

class NodeSelectorProxyModel;
class NodeSelectorModel;
class NavigationBreadcrumb;
class DestinationBreadcrumb;
struct NodeSelectorMergeInfo;
class NodeSelectorTreeViewWidgetCloudDrive;
class NodeSelectorTreeViewWidgetIncomingShares;
class NodeSelectorTreeViewWidgetBackups;
class NodeSelectorTreeViewWidgetSearch;
class NodeSelectorTreeViewWidgetRubbish;
class IncomingShareHeaderWidget;
class DuplicatedNodeDialog;
class QPushButton;
struct ConflictTypes;

struct MessageInfo;

namespace mega
{
class MegaApi;
}

namespace Ui
{
class NodeSelector;
}

class NodeSelector: public QDialog, public mega::MegaListener
{
    Q_OBJECT

public:
    explicit NodeSelector(SelectTypeSPtr selectType, QWidget* parent = 0);
    ~NodeSelector();

    void init();

    void showDefaultUploadOption(bool show = true);
    void setDefaultUploadOption(bool value);
    bool getDefaultUploadOption();
    void setSelectedNodeHandle(std::shared_ptr<mega::MegaNode> node = nullptr);
    mega::MegaHandle findIndexToMoveItem();
    mega::MegaHandle getSelectedNodeHandle() const;
    QList<mega::MegaHandle> getMultiSelectionNodeHandle();
    void closeEvent(QCloseEvent* event) override;
    static void showNotFoundNodeMessageBox();

protected:
    // True while a view is loading/searching (the loading scene is shown). Subclasses use it to
    // hide the search result count until the search finishes.
    bool isUiBlocked() const
    {
        return mUiBlocked;
    }

    // True from the moment a search is launched until it finishes (the loading-scene unblock).
    // Drives the "Searching…" indicator deterministically, without depending on the block-signal
    // timing (which lags on the first search).
    bool isSearchInProgress() const
    {
        return mSearchInProgress;
    }

    // Shows the "Searching…" indicator in the result-count label, used while a search is running
    // (before the final count is known).
    void showSearchingIndicator();

    bool event(QEvent* event) override;
    void changeEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void addBackupsView();
    std::shared_ptr<mega::MegaNode> getSelectedNode() const;
    // Resolves the parent folder for a newly created folder. Base behaviour uses the current
    // navigation root; the file picker overrides it to use the selected folder.
    virtual std::shared_ptr<mega::MegaNode>
        getNewFolderParentNode(NodeSelectorTreeViewWidget* sourceWidget) const;
    // Applies a newly created (or existing-duplicate) folder to the view. The file manager marks it
    // via setNewFolderInfo so it navigates into it once it is added; the file picker reveals and
    // selects the folder directly.
    virtual void applyNewFolderSelection(NodeSelectorTreeViewWidget* sourceWidget,
                                         mega::MegaNode* newNode) = 0;
    void initSpecialisedWidgets(NodeSelectorTreeViewWidget* wid);
    void createActionButtons();
    bool eventFilter(QObject* obj, QEvent* event) override;

    virtual void onRequestFinish(mega::MegaApi* api,
                                 mega::MegaRequest* request,
                                 mega::MegaError* e) override
    {}

    void onNodesUpdate(mega::MegaApi* api, mega::MegaNodeList* nodes) override;

    NodeSelectorTreeViewWidget* getTreeViewWidget(int page) const;
    NodeSelectorTreeViewWidget* getTreeViewWidget(QObject* object) const;
    NodeSelectorTreeViewWidget* getCurrentTreeViewWidget() const;

    // While the ghost search tab is shown, the browsed tab is the one whose search chip is
    // selected. Outside of search it is simply the current tab. Used to resolve the
    // destination breadcrumb root and chip-specific banners.
    NodeSelectorTreeViewWidget* selectedSearchChipTreeViewWidget() const;

    enum class IncreaseOrDecrease
    {
        INCREASE,
        DECREASE
    };

    void performItemsToBeMoved(const QList<mega::MegaHandle>& handles,
                               IncreaseOrDecrease type,
                               bool blockSource,
                               bool blockTarget);

    // Create specialised widgets
    void createSpecialisedTreeViewWidgets();
    NodeSelectorTreeViewWidget* addWidgetForTabType(TabType type);

    // Layout configuration hooks. Default behaviour matches FilePicker (collapsed sidebar,
    // SearchLineEdit in header, action buttons in footer, footer visible).
    // Subclasses override to relocate widgets and tweak properties at runtime.
    virtual void configureSidebar() {}

    virtual void configureHeader() {}

    virtual void configureActionButtonsPlacement() {}

    virtual void configureFooter() {}

    virtual bool shouldClearSelectionOnBackgroundClick(const QPoint& pos) const
    {
        Q_UNUSED(pos);
        return true;
    }

    virtual void specialisedTreeViewWidgetsCreated();

    virtual bool initialShowLabelText() const
    {
        return true;
    }

    virtual void configureTableColumns(NodeSelectorTreeViewWidget* widget);
    virtual void configureTypeSpecificColumns(NodeSelectorTreeViewWidget* widget);
    void setIncomingShareColumnsVisibility(NodeSelectorTreeViewWidget* widget, bool visible);

    virtual void configureSearchWidget(TabType type);

    // Unified search behaviour for every NodeSelector: the search results live in a "ghost"
    // tab (no entry selected in the left sidebar) that spans every chip, with the in-view
    // chips driving which one is shown. handleSearch() shows it; hideGhostSearch() restores
    // the previously visible tab.
    void handleSearch(const QString& text);
    void hideGhostSearch();

    // The active search chip type, or TabType::NONE when the ghost search tab is not shown.
    TabType mActiveSearchTabType = TabType::NONE;
    // The last real tab shown before the ghost search tab, restored when search is dismissed.
    NodeSelectorTreeViewWidget::TabItem mTabBeforeSearch =
        NodeSelectorTreeViewWidget::TabItem::CLOUD_DRIVE;

    virtual void onLanguageChangeEvent() {}

    NodeSelectorTreeViewWidget* addCloudDrive();
    NodeSelectorTreeViewWidget* addIncomingShares();
    NodeSelectorTreeViewWidget* addBackups();
    NodeSelectorTreeViewWidget* addSearchTreeViewWidget();
    NodeSelectorTreeViewWidget* addRubbish();

    NodeSelectorTreeViewWidgetCloudDrive* mCloudDriveWidget = nullptr;
    NodeSelectorTreeViewWidgetIncomingShares* mIncomingSharesWidget = nullptr;
    NodeSelectorTreeViewWidgetBackups* mBackupsWidget = nullptr;
    NodeSelectorTreeViewWidgetSearch* mSearchWidget = nullptr;
    NodeSelectorTreeViewWidgetRubbish* mRubbishWidget = nullptr;

    mega::MegaApi* mMegaApi;
    Ui::NodeSelector* ui;
    SelectTypeSPtr mSelectType;

    QMap<uint, QPushButton*> mButtons;

protected slots:

    virtual void onCustomButtonClicked(uint id);

    virtual void onItemsAboutToBeMoved(const QList<mega::MegaHandle>& /*handles*/,
                                       int /*actionType*/)
    {}

    virtual void onItemsAboutToBeMovedFailed(const QList<mega::MegaHandle>& /*handles*/,
                                             int /*type*/)
    {}

    virtual void onItemRequestsFinished(int actionType) {}

    virtual void onItemsAboutToBeRestored(const QSet<mega::MegaHandle>&) {}

    virtual void onItemAboutToBeReplaced(mega::MegaHandle) {}

    virtual void onItemsAboutToBeMerged(const QList<std::shared_ptr<NodeSelectorMergeInfo>>&, int)
    {}

    virtual void onItemMergeFinished(mega::MegaHandle, mega::MegaHandle, int) {}

    virtual void onItemsAboutToBeMergedFailed(const QList<std::shared_ptr<NodeSelectorMergeInfo>>&,
                                              int)
    {}

    virtual void onModelModified() {}

    void onbShowCloudDriveClicked();
    void onbShowIncomingSharesClicked();
    void onOptionSelected(int index);

    void onCloudDriveTabDropped(std::shared_ptr<QDropEvent> event);

private slots:
    void confirmSelection();
    void onbShowBackupsFolderClicked();
    void onbShowRubbishClicked();
    void updateNodeSelectorTabs();
    void onCurrentTreeViewWidgetChanged(int index);
    void onShowDuplicatedNodeDialog(QPointer<DuplicatedNodeDialog>);
    void performNodeSelection();
    void onSearch(const QString& text);
    void onUiIsBlocked(bool state);
    void onSelectionChanged();
    void onbNewFolderClicked();

private:
    using ViewConfigurationFunction = void (NodeSelector::*)(NodeSelectorTreeViewWidget*);

    NodeSelectorTreeViewWidget* widgetForTab(NodeSelectorTreeViewWidget::TabItem item) const;
    NodeSelectorTreeViewWidget* widgetForTabType(TabType type) const;
    std::optional<NodeSelectorTreeViewWidget::TabItem>
        tabItemForWidget(const NodeSelectorTreeViewWidget* wid) const;
    void showTab(NodeSelectorTreeViewWidget::TabItem item);
    void connectViewConfiguration(NodeSelectorTreeViewWidget* widget,
                                  ViewConfigurationFunction configure);
    QString folderNameForWidget(NodeSelectorTreeViewWidget* wid) const;
    void applyHeaderFolderInfoState(NodeSelectorTreeViewWidget* wid);
    void applyHeaderButtonsState(NodeSelectorTreeViewWidget* wid);
    void applySearchToolVisibilityState(NodeSelectorTreeViewWidget* wid,
                                        NodeSelectorTreeViewWidget::ViewType type);
    void updateHeaderTopRowVisibility();
    void refreshHeader(NodeSelectorTreeViewWidget* wid);
    void refreshHeaderButtons(NodeSelectorTreeViewWidget* wid);
    void updateOkButtonState(NodeSelectorTreeViewWidget* wid);
    void refreshBreadcrumbs();

    // Connections to the current tree view widget. Register every per-current-widget
    // connection here so they are all torn down together when the current widget changes.
    void addCurrentWidgetConnection(const QMetaObject::Connection& connection);
    void disconnectCurrentWidgetConnections();

    virtual void refreshDestinationBreadcrumb() {}

    // The navigation area changed (tab switch, root navigation). Subclasses that show a navigation
    // path override this: CloudDrive populates the NavigationBreadcrumb, the file picker updates a
    // read-only top-root label.
    virtual void refreshNavigationBreadcrumb() {}

    // A batch of nodes was renamed. Subclasses with a navigation path forward the handles to their
    // breadcrumb so it can refresh if any belongs to the current path.
    virtual void onNodesRenamed(const QList<mega::MegaHandle>&) {}

    virtual void refreshSearchResultCount() {}

    virtual void onOkButtonClicked() = 0;
    void shortCutConnects(int ignoreThis);
    void resetButtonsText();

    std::optional<NodeSelectorTreeViewWidget::TabItem> selectedNodeTab();

    std::unique_ptr<mega::QTMegaListener> mDelegateListener;

    bool mManuallyResizedColumn;
    bool mInitialised;
    bool mUiBlocked = false;
    bool mSearchInProgress = false;

    std::shared_ptr<mega::MegaNode> mNodeToBeSelected;

    // Duplicated node details
    std::shared_ptr<ConflictTypes> mDuplicatedConflicts;
    std::optional<int> mDuplicatedType;
    NodeSelectorModel* mDuplicatedModel;

    // Loading view
    QList<NodeSelectorTreeViewWidget*> mSourceWids;
    NodeSelectorTreeViewWidget* mTargetWid;

    // Selection changed signal
    QList<QMetaObject::Connection> mCurrentWidgetConnections;
};

#endif // NODESELECTOR_H
