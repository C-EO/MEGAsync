#ifndef NODESELECTORTREEVIEWWIDGETSPECIALIZATIONS_H
#define NODESELECTORTREEVIEWWIDGETSPECIALIZATIONS_H

#include "NodeSelectorProxyModel.h"
#include "NodeSelectorSearchController.h"
#include "NodeSelectorTreeViewWidget.h"

#include <QLabel>
#include <QModelIndex>
#include <QPointer>
#include <QToolButton>
#include <QWidget>

#include <memory>
#include <optional>

class NodeSelectorModel;
class NodeSelectorModelSearch;
class NodeSelectorProxyModelSearch;
class NodeSearchRowDelegate;
class RestoreNodeManager;
class TabSelector;

class NodeSelectorTreeViewWidgetCloudDrive: public NodeSelectorTreeViewWidget
{
    Q_OBJECT

public:
    explicit NodeSelectorTreeViewWidgetCloudDrive(SelectTypeSPtr mode, QWidget* parent = nullptr);

    void setShowEmptyView(bool newShowEmptyView);

protected:
    bool isNodeCompatibleWithModel(mega::MegaNode* node) override;

private:
    QString getRootText() override;
    std::unique_ptr<NodeSelectorModel> createModel() override;
    void setViewPage() override;
    QIcon getEmptyIcon() override;
    EmptyLabelInfo getEmptyLabel() override;

    bool showEmptyView() override
    {
        return mShowEmptyView;
    }

    bool isCurrentRootIndexReadOnly() const override;

    mega::MegaHandle findMergedSibling(std::shared_ptr<mega::MegaNode> node);

    bool mShowEmptyView = true;
    QList<mega::MegaHandle> mRestoredHandles;
};

class NodeSelectorTreeViewWidgetIncomingShares: public NodeSelectorTreeViewWidget
{
    Q_OBJECT

public:
    explicit NodeSelectorTreeViewWidgetIncomingShares(SelectTypeSPtr mode,
                                                      QWidget* parent = nullptr);

signals:
    void incomingShareAccessChanged();

protected:
    bool isNodeCompatibleWithModel(mega::MegaNode* node) override;
    std::optional<IncomingShareHeaderData> incomingShareHeaderData() const override;
    void makeViewConnections() override;

private:
    QString getRootText() override;
    std::unique_ptr<NodeSelectorModel> createModel() override;
    bool isCurrentRootIndexReadOnly() const override;
    bool isSelectionReadOnly(const QModelIndexList& selection) override;
    bool isCurrentSelectionReadOnly() override;
    QIcon getEmptyIcon() override;
    EmptyLabelInfo getEmptyLabel() override;
};

class NodeSelectorTreeViewWidgetBackups: public NodeSelectorTreeViewWidget
{
    Q_OBJECT

public:
    explicit NodeSelectorTreeViewWidgetBackups(SelectTypeSPtr mode, QWidget* parent = nullptr);

private:
    QString getRootText() override;
    std::unique_ptr<NodeSelectorModel> createModel() override;

    bool isCurrentRootIndexReadOnly() const override
    {
        return true;
    }

    bool isCurrentSelectionReadOnly() override
    {
        return true;
    }

    bool isSelectionReadOnly(const QModelIndexList&) override
    {
        return true;
    }

    QIcon getEmptyIcon() override;
    EmptyLabelInfo getEmptyLabel() override;
};

class NodeSelectorTreeViewWidgetSearch: public NodeSelectorTreeViewWidget
{
    Q_OBJECT

public:
    explicit NodeSelectorTreeViewWidgetSearch(SelectTypeSPtr mode, QWidget* parent = nullptr);
    void prepareForInitialDisplay();
    void resetSearchState();
    void search(const QString& text);
    void stopSearch();
    void setSearchScope(std::optional<TabType> scope);
    bool isCurrentRootIndexReadOnly() const override;
    bool isSelectionReadOnly(const QModelIndexList& selection) override;

    std::shared_ptr<RestoreNodeManager> getRestoreManager() const;

public slots:
    void resetMovingNumber();
    void setViewPage() override;

signals:
    void nodeDoubleClicked(std::shared_ptr<mega::MegaNode> node, bool goToInit);
    void searchCounterChanged();
    void searchTabTypeChanged(TabType type);

protected:
    bool isNodeCompatibleWithModel(mega::MegaNode* node) override;
    QModelIndex getAddedNodeParent(mega::MegaHandle parentHandle) override;
    void makeCustomConnections() override;
    void onLevelLoaded() override;

protected slots:
    NodeState getNodeOnModelState(const QModelIndex& index, mega::MegaNode* node) override;

private slots:
    void onItemDoubleClick(const QModelIndex& index) override;

private:
    void onSearchTabClicked(TabType type);
    void changeColumnsVisibility(TabType type);
    void expandSearchResults();
    QString getRootText() override;
    std::shared_ptr<NodeSelectorProxyModel> createProxyModel() override;
    std::unique_ptr<NodeSelectorModel> createModel() override;
    QIcon getEmptyIcon() override;
    EmptyLabelInfo getEmptyLabel() override;
    NodeSelectorDelegate* createItemDelegate(QObject* parent) override;
    NodeSelectorModelSearch* searchModel() const;
    NodeSelectorProxyModelSearch* searchProxyModel() const;

    std::unique_ptr<NodeSelectorSearchController> mSearchController;
    std::shared_ptr<RestoreNodeManager> mRestoreManager;
    QPointer<NodeSearchRowDelegate> mSearchDelegate;
};

class NodeSelectorTreeViewWidgetRubbish: public NodeSelectorTreeViewWidget
{
    Q_OBJECT

public:
    explicit NodeSelectorTreeViewWidgetRubbish(SelectTypeSPtr mode, QWidget* parent = nullptr);
    void setShowEmptyView(bool newShowEmptyView);
    bool isEmpty() const;

protected:
    bool isNodeCompatibleWithModel(mega::MegaNode* node) override;
    void makeViewConnections() override;

private:
    QString getRootText() override;
    std::unique_ptr<NodeSelectorModel> createModel() override;
    void setViewPage() override;
    QIcon getEmptyIcon() override;
    EmptyLabelInfo getEmptyLabel() override;

    bool showEmptyView() override
    {
        return mShowEmptyView;
    }

    bool isCurrentRootIndexReadOnly() const override
    {
        return true;
    }

    bool isCurrentSelectionReadOnly() override
    {
        return true;
    }

    bool isSelectionReadOnly(const QModelIndexList&) override
    {
        return true;
    }

    bool mShowEmptyView = true;

    std::shared_ptr<RestoreNodeManager> mRestoreManager;
};

#endif // NODESELECTORTREEVIEWWIDGETSPECIALIZATIONS_H
