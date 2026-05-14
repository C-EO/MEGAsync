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
#include <QTimer>

#include <memory>
#include <optional>

class NodeSelectorProxyModel;
class NodeSelectorModel;
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
    enum TabItem
    {
        CLOUD_DRIVE = 0,
        SHARES,
        BACKUPS,
        RUBBISH,
        SEARCH
    };
    Q_ENUM(TabItem)

    enum class ClearType
    {
        CLEAR_ON_CLOSE_SEARCH_TAB = 0x0,
        CLEAR_ON_CLEAR_SEARCH_LINE_EDIT = 0x1,
        CLEAR_ON_TAB_CHANGE = 0x2
    };
    Q_DECLARE_FLAGS(ClearTypes, ClearType)

    explicit NodeSelector(SelectTypeSPtr selectType, QWidget* parent = 0);
    ~NodeSelector();

    void init();

    void showDefaultUploadOption(bool show = true);
    void setDefaultUploadOption(bool value);
    bool getDefaultUploadOption();
    void setSelectedNodeHandle(std::shared_ptr<mega::MegaNode> node = nullptr);
    mega::MegaHandle findIndexToMoveItem();
    mega::MegaHandle getSelectedNodeHandle();
    QList<mega::MegaHandle> getMultiSelectionNodeHandle();
    void closeEvent(QCloseEvent* event) override;
    static void showNotFoundNodeMessageBox();

protected:
    bool event(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void addBackupsView();
    std::shared_ptr<mega::MegaNode> getSelectedNode();
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
    NodeSelectorTreeViewWidget* getSearchAwareTargetWidget() const;

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

    virtual void configureSearchTool() {}

    virtual void configureActionButtonsPlacement() {}

    virtual void configureFooterVisibility() {}

    virtual void specialisedTreeViewWidgetsCreated();

    virtual void configureCloudDriveWidget() {}

    virtual void configureIncomingSharesWidget() {}

    virtual void configureBackupsWidget() {}

    virtual void configureRubbishWidget() {}

    virtual void configureSearchWidget(TabType type)
    {
        Q_UNUSED(type);
    }

    virtual ClearTypes searchClearType() const;
    virtual bool searchHasOwnTab() const = 0;

    virtual void handleSearch(const QString& /*text*/) {}

    virtual void handleSearchHidden() {}

    bool isCurrentTabSearchActive() const;
    std::optional<TabItem> currentSearchSourceTab() const;
    void clearCurrentTabSearch(bool clearLineEdit);
    TabType tabTypeForItem(TabItem item) const;

    std::optional<TabItem> mSearchSourceTab;
    QString mLastSearchText;

    virtual void onLanguageChangeEvent() {}

    NodeSelectorTreeViewWidget* addCloudDrive();
    NodeSelectorTreeViewWidget* addIncomingShares();
    NodeSelectorTreeViewWidget* addBackups();
    NodeSelectorTreeViewWidget* addSearchTreeViewWidget();
    NodeSelectorTreeViewWidget* addRubbish();

    NodeSelectorTreeViewWidgetCloudDrive* mCloudDriveWidget;
    NodeSelectorTreeViewWidgetIncomingShares* mIncomingSharesWidget;
    NodeSelectorTreeViewWidgetBackups* mBackupsWidget;
    NodeSelectorTreeViewWidgetSearch* mSearchWidget;
    NodeSelectorTreeViewWidgetRubbish* mRubbishWidget;

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
    NodeSelectorTreeViewWidget* widgetForTab(TabItem item) const;
    std::optional<TabItem> tabItemForWidget(const NodeSelectorTreeViewWidget* wid) const;
    void showTab(TabItem item);
    QString folderNameForWidget(NodeSelectorTreeViewWidget* wid) const;
    void applyHeaderFolderInfoState(NodeSelectorTreeViewWidget* wid);
    void applyNavigationButtonsState(NodeSelectorTreeViewWidget* wid);
    void applyHeaderButtonsState(NodeSelectorTreeViewWidget* wid);
    void refreshHeader(NodeSelectorTreeViewWidget* wid);
    void refreshHeaderButtons(NodeSelectorTreeViewWidget* wid);

    virtual void onOkButtonClicked() = 0;
    void shortCutConnects(int ignoreThis);
    void resetButtonsText();

    std::optional<TabItem> selectedNodeTab();

    std::unique_ptr<mega::QTMegaListener> mDelegateListener;

    bool mManuallyResizedColumn;
    bool mInitialised;

    std::shared_ptr<mega::MegaNode> mNodeToBeSelected;

    // Duplicated node details
    std::shared_ptr<ConflictTypes> mDuplicatedConflicts;
    std::optional<int> mDuplicatedType;
    NodeSelectorModel* mDuplicatedModel;

    // Loading view
    QList<NodeSelectorTreeViewWidget*> mSourceWids;
    NodeSelectorTreeViewWidget* mTargetWid;

    // Selection changed signal
    QMetaObject::Connection mSelectionChangedConnection;
    QMetaObject::Connection mViewStateConnection;
    QMetaObject::Connection mViewButtonsStateConnection;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(NodeSelector::ClearTypes)

#endif // NODESELECTOR_H
