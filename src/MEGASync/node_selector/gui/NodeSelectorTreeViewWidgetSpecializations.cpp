#include "NodeSelectorTreeViewWidgetSpecializations.h"

#include "DeviceNames.h"
#include "MegaNodeNames.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorModelSpecialised.h"
#include "NodeSelectorProxyModel.h"
#include "RequestListenerManager.h"
#include "RestoreNodeManager.h"
#include "ui_NodeSelectorTreeViewWidget.h"
#include <MegaApplication.h>

///////////////////////////////////////////////////////////////////
NodeSelectorTreeViewWidgetCloudDrive::NodeSelectorTreeViewWidgetCloudDrive(SelectTypeSPtr mode,
                                                                           QWidget* parent):
    NodeSelectorTreeViewWidget(mode, parent)
{}

void NodeSelectorTreeViewWidgetCloudDrive::setShowEmptyView(bool newShowEmptyView)
{
    mShowEmptyView = newShowEmptyView;
}

bool NodeSelectorTreeViewWidgetCloudDrive::isNodeCompatibleWithModel(mega::MegaNode* node)
{
    return MegaSyncApp->getMegaApi()->isInCloud(node);
}

QString NodeSelectorTreeViewWidgetCloudDrive::getRootText()
{
    return MegaNodeNames::getCloudDriveName();
}

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetCloudDrive::createModel()
{
    return std::unique_ptr<NodeSelectorModelCloudDrive>(new NodeSelectorModelCloudDrive);
}

void NodeSelectorTreeViewWidgetCloudDrive::setViewPage()
{
    if (mProxyModel->rowCount(getCurrentRootIndex()) == 0 && showEmptyView())
    {
        ui->stackedWidget->setCurrentWidget(ui->emptyPage);
    }
    else
    {
        ui->stackedWidget->setCurrentWidget(ui->treeViewPage);
    }
}

QIcon NodeSelectorTreeViewWidgetCloudDrive::getEmptyIcon()
{
    return Utilities::getIcon(QLatin1String("cloud"),
                              Utilities::AttributeType::SMALL | Utilities::AttributeType::THIN |
                                  Utilities::AttributeType::OUTLINE);
}

NodeSelectorTreeViewWidget::EmptyLabelInfo NodeSelectorTreeViewWidgetCloudDrive::getEmptyLabel()
{
    EmptyLabelInfo info;
    info.description = tr("Cloud drive is empty");
    return info;
}

bool NodeSelectorTreeViewWidgetCloudDrive::isCurrentRootIndexReadOnly()
{
    return false;
}

MegaHandle
    NodeSelectorTreeViewWidgetCloudDrive::findMergedSibling(std::shared_ptr<mega::MegaNode> node)
{
    std::unique_ptr<mega::MegaNode> parentNode(
        MegaSyncApp->getMegaApi()->getParentNode(node.get()));
    if (parentNode)
    {
        std::unique_ptr<mega::MegaNode> foundNode(
            MegaSyncApp->getMegaApi()->getChildNode(parentNode.get(), node->getName()));
        if (foundNode)
        {
            return foundNode->getHandle();
        }
    }

    return mega::INVALID_HANDLE;
}

void NodeSelectorTreeViewWidgetCloudDrive::onRootIndexChanged(const QModelIndex& source_idx)
{
    Q_UNUSED(source_idx)
    ui->tMegaFolders->header()->hideSection(NodeSelectorModel::Column::USER);
    ui->tMegaFolders->header()->hideSection(NodeSelectorModel::Column::ACCESS);

    NodeSelectorTreeViewWidget::onRootIndexChanged(source_idx);
}

/////////////////////////////////////////////////////////////////
/// \brief NodeSelectorTreeViewWidgetIncomingShares::NodeSelectorTreeViewWidgetIncomingShares
/// \param mode
/// \param parent

NodeSelectorTreeViewWidgetIncomingShares::NodeSelectorTreeViewWidgetIncomingShares(
    SelectTypeSPtr mode,
    QWidget* parent):
    NodeSelectorTreeViewWidget(mode, parent)
{}

bool NodeSelectorTreeViewWidgetIncomingShares::isNodeCompatibleWithModel(mega::MegaNode* node)
{
    if (node->isInShare())
    {
        return true;
    }

    auto access(Utilities::getNodeAccess(node));

    return access != mega::MegaShare::ACCESS_OWNER && access != mega::MegaShare::ACCESS_UNKNOWN;
}

std::optional<IncomingShareHeaderData>
    NodeSelectorTreeViewWidgetIncomingShares::incomingShareHeaderData() const
{
    const auto rootIndex = getCurrentRootIndex();
    if (!rootIndex.isValid())
    {
        return std::nullopt;
    }

    IncomingShareHeaderData incomingInfo;
    incomingInfo.folderName = rootIndex.data(Qt::DisplayRole).toString();
    incomingInfo.folderIcon = qvariant_cast<QPixmap>(rootIndex.data(Qt::DecorationRole));

    auto inShareIndex = getParentIncomingShareByIndex(rootIndex);
    auto item = NodeSelectorModel::getItemByIndex(inShareIndex);
    if (inShareIndex.isValid() && item)
    {
        inShareIndex = inShareIndex.sibling(inShareIndex.row(), NodeSelectorModel::Column::USER);
        incomingInfo.userIcon = qvariant_cast<QPixmap>(inShareIndex.data(Qt::DecorationRole));

        inShareIndex = inShareIndex.sibling(inShareIndex.row(), NodeSelectorModel::Column::ACCESS);
        incomingInfo.accessIcon = qvariant_cast<QPixmap>(inShareIndex.data(Qt::DecorationRole));
        incomingInfo.accessLabel = inShareIndex.data(Qt::DisplayRole).toString();
        incomingInfo.accessType =
            inShareIndex.data(toInt(NodeSelectorModelRoles::ACCESS_ROLE)).toInt();
        incomingInfo.userEmail = item->getOwnerEmail();
        incomingInfo.userName = item->getOwnerName();
    }

    return incomingInfo;
}

void NodeSelectorTreeViewWidgetIncomingShares::makeViewConnections()
{
    auto incomingSharesModel = qobject_cast<NodeSelectorModelIncomingShares*>(mModel.get());
    if (!incomingSharesModel)
    {
        return;
    }

    connect(
        incomingSharesModel,
        &NodeSelectorModelIncomingShares::incomingShareInfoChanged,
        this,
        [this](mega::MegaHandle handle)
        {
            const auto rootIndex = getCurrentRootIndex();
            if (!rootIndex.isValid())
            {
                return;
            }

            if (getHandleByIndex(rootIndex) == handle)
            {
                notifyViewStateChanged();
                return;
            }

            const auto incomingShareIndex = getParentIncomingShareByIndex(rootIndex);
            if (incomingShareIndex.isValid() && getHandleByIndex(incomingShareIndex) == handle)
            {
                notifyViewStateChanged();
            }
        },
        Qt::UniqueConnection);
}

QString NodeSelectorTreeViewWidgetIncomingShares::getRootText()
{
    return MegaNodeNames::getIncomingSharesName();
}

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetIncomingShares::createModel()
{
    return std::unique_ptr<NodeSelectorModelIncomingShares>(new NodeSelectorModelIncomingShares);
}

void NodeSelectorTreeViewWidgetIncomingShares::onRootIndexChanged(const QModelIndex& idx)
{
    if (idx.isValid())
    {
        ui->tMegaFolders->header()->hideSection(NodeSelectorModel::Column::USER);
        ui->tMegaFolders->header()->hideSection(NodeSelectorModel::Column::ACCESS);
    }
    else
    {
        ui->tMegaFolders->header()->showSection(NodeSelectorModel::Column::USER);
        ui->tMegaFolders->header()->showSection(NodeSelectorModel::Column::ACCESS);
    }

    ui->tMegaFolders->header()->hideSection(NodeSelectorModel::Column::ADDED_DATE);

    NodeSelectorTreeViewWidget::onRootIndexChanged(idx);
}

bool NodeSelectorTreeViewWidgetIncomingShares::isCurrentRootIndexReadOnly()
{
    auto rootIndex(ui->tMegaFolders->rootIndex());
    if (rootIndex.isValid())
    {
        auto rootNode = mProxyModel->getNode(rootIndex);
        if (rootNode)
        {
            return MegaSyncApp->getMegaApi()->getAccess(rootNode.get()) <=
                   mega::MegaShare::ACCESS_READ;
        }
    }

    return true;
}

bool NodeSelectorTreeViewWidgetIncomingShares::isSelectionReadOnly(const QModelIndexList& selection)
{
    bool anyReadOnly(false);

    foreach(auto index, selection)
    {
        auto rootIndex(getRootIndexFromIndex(index));
        if (rootIndex.isValid())
        {
            auto rootNode = mProxyModel->getNode(rootIndex);
            if (rootNode)
            {
                if (MegaSyncApp->getMegaApi()->getAccess(rootNode.get()) <=
                    mega::MegaShare::ACCESS_READ)
                {
                    anyReadOnly = true;
                    break;
                }
            }
        }
    }

    return anyReadOnly;
}

bool NodeSelectorTreeViewWidgetIncomingShares::isCurrentSelectionReadOnly()
{
    return isSelectionReadOnly(ui->tMegaFolders->selectedRows());
}

QIcon NodeSelectorTreeViewWidgetIncomingShares::getEmptyIcon()
{
    return Utilities::getIcon(QLatin1String("folder-users"),
                              Utilities::AttributeType::SMALL | Utilities::AttributeType::THIN |
                                  Utilities::AttributeType::OUTLINE);
}

NodeSelectorTreeViewWidget::EmptyLabelInfo NodeSelectorTreeViewWidgetIncomingShares::getEmptyLabel()
{
    EmptyLabelInfo info;
    if (std::dynamic_pointer_cast<SyncType>(mSelectType))
    {
        info.title = tr("No incoming shares you can sync");
        info.description = tr("You can only sync a shared folder if you’ve been given full access");
    }
    else
    {
        info.description = tr("No incoming shares");
    }

    return info;
}

/////////////////////////////////////////////////////////////////
NodeSelectorTreeViewWidgetBackups::NodeSelectorTreeViewWidgetBackups(SelectTypeSPtr mode,
                                                                     QWidget* parent):
    NodeSelectorTreeViewWidget(mode, parent)
{
    // Monitor Device Names changes and update the title if the current folder is a Device Folder
    auto deviceNamesRequest = UserAttributes::DeviceNames::requestDeviceNames();
    connect(deviceNamesRequest.get(),
            &UserAttributes::DeviceNames::attributeReady,
            this,
            [&]()
            {
                auto rootIndex(ui->tMegaFolders->rootIndex());
                auto* item = NodeSelectorModel::getItemByIndex(rootIndex);
                if (item && item->isDeviceFolder())
                {
                    notifyViewStateChanged();
                }
            });
}

QString NodeSelectorTreeViewWidgetBackups::getRootText()
{
    return MegaNodeNames::getBackupsName();
}

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetBackups::createModel()
{
    return std::unique_ptr<NodeSelectorModelBackups>(new NodeSelectorModelBackups);
}

QIcon NodeSelectorTreeViewWidgetBackups::getEmptyIcon()
{
    return Utilities::getIcon(QLatin1String("devices"),
                              Utilities::AttributeType::SMALL | Utilities::AttributeType::THIN |
                                  Utilities::AttributeType::OUTLINE);
}

NodeSelectorTreeViewWidget::EmptyLabelInfo NodeSelectorTreeViewWidgetBackups::getEmptyLabel()
{
    EmptyLabelInfo info;
    info.description = tr("No backups");
    return info;
}

void NodeSelectorTreeViewWidgetBackups::onRootIndexChanged(const QModelIndex& idx)
{
    ui->tMegaFolders->header()->hideSection(NodeSelectorModel::Column::USER);
    ui->tMegaFolders->header()->hideSection(NodeSelectorModel::Column::ACCESS);

    NodeSelectorTreeViewWidget::onRootIndexChanged(idx);
}

/////////////////////////////////////////////////////////////////

NodeSelectorTreeViewWidgetSearch::NodeSelectorTreeViewWidgetSearch(SelectTypeSPtr mode,
                                                                   QWidget* parent):
    NodeSelectorTreeViewWidget(mode, parent)
{
    mSearchController = std::make_unique<NodeSelectorSearchController>(ui);

    connect(ui->cloudDriveSearch,
            &TabSelector::clicked,
            this,
            [this]()
            {
                onSearchTabClicked(TabType::CLOUD_DRIVE);
            });

    connect(ui->incomingSharesSearch,
            &TabSelector::clicked,
            this,
            [this]()
            {
                onSearchTabClicked(TabType::INCOMING_SHARE);
            });

    connect(ui->backupsSearch,
            &TabSelector::clicked,
            this,
            [this]()
            {
                onSearchTabClicked(TabType::BACKUP);
            });

    connect(ui->rubbishSearch,
            &TabSelector::clicked,
            this,
            [this]()
            {
                onSearchTabClicked(TabType::RUBBISH);
            });

    ui->tMegaFolders->loadingView().setDelayTimeToShowInMs(0);
}

void NodeSelectorTreeViewWidgetSearch::prepareForInitialDisplay()
{
    // The search view does not load content on startup. Pre-attaching the view avoids the
    // first visible search having to build the tree view and delegates mid-transition.
    setLoadingSceneVisible(false);
    onExpandReady();
}

void NodeSelectorTreeViewWidgetSearch::setSearchScope(std::optional<TabType> scope)
{
    mSearchController->setSearchScope(scope);
}

void NodeSelectorTreeViewWidgetSearch::resetSearchState()
{
    if (auto model = searchModel())
    {
        model->stopSearch();
    }

    mSearchController->resetSearch();
}

void NodeSelectorTreeViewWidgetSearch::search(const QString& text)
{
    mSearchController->beginSearch(text);
    setStyleSheet(styleSheet());

    if (auto model = searchModel())
    {
        model->setAllowedTabTypes(
            mSearchController->allowedTypes(getSelectType()->allowedTabTypes()));
        model->searchByText(text);
    }
}

void NodeSelectorTreeViewWidgetSearch::stopSearch()
{
    if (auto model = searchModel())
    {
        model->stopSearch();
    }
    mSearchController->stopSearch();
}

std::shared_ptr<NodeSelectorProxyModel> NodeSelectorTreeViewWidgetSearch::createProxyModel()
{
    auto proxy =
        std::make_shared<NodeSelectorProxyModelSearch>(getSelectType()->createProxyModel());

    // The search view is the only one with a real proxy model (in terms on filterAcceptsRow)
    connect(proxy.get(),
            &QAbstractItemModel::rowsInserted,
            this,
            &NodeSelectorTreeViewWidget::checkViewOnModelChange);
    connect(proxy.get(),
            &NodeSelectorProxyModelSearch::modeEmpty,
            this,
            &NodeSelectorTreeViewWidget::checkViewOnModelChange);

    return proxy;
}

bool NodeSelectorTreeViewWidgetSearch::isCurrentRootIndexReadOnly()
{
    return true;
}

bool NodeSelectorTreeViewWidgetSearch::isSelectionReadOnly(const QModelIndexList& selection)
{
    return true;
}

void NodeSelectorTreeViewWidgetSearch::resetMovingNumber()
{
    mModel->finishMovingNodes();
}

bool NodeSelectorTreeViewWidgetSearch::isNodeCompatibleWithModel(mega::MegaNode* node)
{
    return mSearchController->matchesNodeName(node);
}

QModelIndex NodeSelectorTreeViewWidgetSearch::getAddedNodeParent(mega::MegaHandle parentHandle)
{
    Q_UNUSED(parentHandle)
    return QModelIndex();
}

void NodeSelectorTreeViewWidgetSearch::makeViewConnections()
{
    connect(ui->tMegaFolders,
            &NodeSelectorTreeView::restoreClicked,
            mRestoreManager.get(),
            &RestoreNodeManager::onRestoreClicked);
}

NodeSelectorTreeViewWidget::NodeState
    NodeSelectorTreeViewWidgetSearch::getNodeOnModelState(const QModelIndex& index,
                                                          mega::MegaNode* node)
{
    NodeState result(NodeState::DOESNT_EXIST);

    if (mSearchController->hasRows() && node)
    {
        if (index.isValid())
        {
            result = NodeState::EXISTS;
        }
        else if (isNodeCompatibleWithModel(node))
        {
            result = NodeState::ADD;
        }
    }

    return result;
}

void NodeSelectorTreeViewWidgetSearch::onSearchTabClicked(TabType type)
{
    mSearchController->activateMode(
        type,
        searchProxyModel(),
        [this](TabType t)
        {
            changeColumnsVisibility(t);
        },
        [this]()
        {
            expandSearchResults();
        });
}

void NodeSelectorTreeViewWidgetSearch::onItemDoubleClick(const QModelIndex& index)
{
    auto node = qvariant_cast<std::shared_ptr<MegaNode>>(
        index.data(toInt(NodeSelectorModelRoles::NODE_ROLE)));
    emit nodeDoubleClicked(node, true);
}

void NodeSelectorTreeViewWidgetSearch::changeColumnsVisibility(TabType type)
{
    bool hideUserColumn(true);
    bool hideAccessColumn(true);
    bool hideAddedDate(true);

    switch (type)
    {
        case TabType::BACKUP:
        case TabType::CLOUD_DRIVE:
        {
            hideAddedDate = false;
            break;
        }
        case TabType::INCOMING_SHARE:
        {
            hideUserColumn = false;
            hideAccessColumn = false;
            break;
        }
        case TabType::RUBBISH:
        case TabType::NONE:
        default:
            break;
    }

    ui->tMegaFolders->setColumnHidden(NodeSelectorModel::Column::USER, hideUserColumn);
    ui->tMegaFolders->setColumnHidden(NodeSelectorModel::Column::ACCESS, hideAccessColumn);
    ui->tMegaFolders->setColumnHidden(NodeSelectorModel::Column::ADDED_DATE, hideAddedDate);

    onRootIndexChanged(QModelIndex());
}

void NodeSelectorTreeViewWidgetSearch::expandSearchResults()
{
    ui->tMegaFolders->expandAll();
}

void NodeSelectorTreeViewWidgetSearch::onLevelLoaded()
{
    mSearchController->handleExpandReady(
        searchModel(),
        searchProxyModel(),
        ui->tMegaFolders->model() != nullptr,
        [this]()
        {
        NodeSelectorTreeViewWidget::onExpandReady();
        },
        [this](TabType type)
        {
            if (tabSelected != TabType::NONE)
            {
                return;
            }

            auto proxy_model = static_cast<NodeSelectorProxyModelSearch*>(mProxyModel.get());
            if (searchedTypes.testFlag(type))
            {
                tabSelected = type;
                // If it is the first time we load the model, the base method will run a sort/filter
                // action, so we don´t need to do it, just set the mode without sorting/filtering
                if (ui->tMegaFolders->model() == nullptr)
                {
                    // Block the chip signals to avoid calling the slot, which will sort/filter the
                    // model
                    TabSelector::applyActionToTabSelectors(ui->searchButtonsWidget,
                                                           [](TabSelector* tab)
                                                           {
                                                               tab->blockSignals(true);
                                                           });
                    TabSelector::selectTabIf(ui->searchButtonsWidget,
                                             TAB_TYPE,
                                             static_cast<int>(type));
                    TabSelector::applyActionToTabSelectors(ui->searchButtonsWidget,
                                                           [](TabSelector* tab)
                                                           {
                                                               tab->blockSignals(false);
                                                           });
                    proxy_model->setMode(type, false);
                }
                // If it is not the first time, we just click on the correct chip, and it will
                // sort/filter for us
                else
                {
                    TabSelector::selectTabIf(ui->searchButtonsWidget,
                                             TAB_TYPE,
                                             static_cast<int>(type));
                }
            }
        };

        // Only one will be set, in this order
        setMode(TabType::CLOUD_DRIVE);
        setMode(TabType::INCOMING_SHARE);
        setMode(TabType::BACKUP);
        setMode(TabType::RUBBISH);

        mNewSearch = false;

        NodeSelectorTreeViewWidget::onLevelLoaded();

        // Do it after setting the model to the view, otherwise it won´t work
        changeColumnsVisibility(tabSelected);
        expandSearchResults();
    }
    else
    {
        NodeSelectorTreeViewWidget::onLevelLoaded();
        expandSearchResults();
    }
}

QString NodeSelectorTreeViewWidgetSearch::getRootText()
{
    auto resultNumber(mModel->rowCount());
    QString resultString(tr("%n result found", "", resultNumber));
    return resultString;
}

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetSearch::createModel()
{
    auto model = std::unique_ptr<NodeSelectorModelSearch>(
        new NodeSelectorModelSearch(getSelectType()->allowedTabTypes()));

    connect(model.get(),
            &NodeSelectorModelSearch::nodeTypeHasChanged,
            this,
            &NodeSelectorTreeViewWidgetSearch::setViewPage);

    mRestoreManager = std::make_shared<RestoreNodeManager>(model.get(), this);

    // Detect if the row count changed
    connect(model.get(),
            &QAbstractItemModel::rowsInserted,
            this,
            &NodeSelectorTreeViewWidget::notifyButtonsStateChanged);

    connect(model.get(),
            &QAbstractItemModel::rowsRemoved,
            this,
            &NodeSelectorTreeViewWidget::notifyButtonsStateChanged);

    connect(model.get(),
            &QAbstractItemModel::modelReset,
            this,
            &NodeSelectorTreeViewWidget::notifyButtonsStateChanged);

    return model;
}

QIcon NodeSelectorTreeViewWidgetSearch::getEmptyIcon()
{
    return Utilities::getIcon(QLatin1String("search"),
                              Utilities::AttributeType::SMALL | Utilities::AttributeType::THIN |
                                  Utilities::AttributeType::OUTLINE);
}

NodeSelectorTreeViewWidget::EmptyLabelInfo NodeSelectorTreeViewWidgetSearch::getEmptyLabel()
{
    EmptyLabelInfo info;
    info.description = tr("No search results");
    return info;
}

void NodeSelectorTreeViewWidgetSearch::setViewPage()
{
    if (!mModel)
    {
        return;
    }

    NodeSelectorTreeViewWidget::setViewPage();

    if (ui->tMegaFolders->model())
    {
        const bool hasRows = ui->tMegaFolders->model()->rowCount() > 0;
        mSearchController->setHasRows(hasRows);
        if (!hasRows && showEmptyView())
        {
            ui->stackedWidget->setCurrentWidget(ui->emptyPage);
            return;
        }
    }
    ui->stackedWidget->setCurrentWidget(ui->treeViewPage);
}

NodeSelectorModelSearch* NodeSelectorTreeViewWidgetSearch::searchModel() const
{
    return static_cast<NodeSelectorModelSearch*>(mModel.get());
}

NodeSelectorProxyModelSearch* NodeSelectorTreeViewWidgetSearch::searchProxyModel() const
{
    return static_cast<NodeSelectorProxyModelSearch*>(mProxyModel.get());
}

///////////////////////
NodeSelectorTreeViewWidgetRubbish::NodeSelectorTreeViewWidgetRubbish(SelectTypeSPtr mode,
                                                                     QWidget* parent):
    NodeSelectorTreeViewWidget(mode, parent)
{}

void NodeSelectorTreeViewWidgetRubbish::setShowEmptyView(bool newShowEmptyView)
{
    mShowEmptyView = newShowEmptyView;
}

bool NodeSelectorTreeViewWidgetRubbish::isEmpty() const
{
    auto rootIndex = mModel->index(0, 0);
    return mModel->rowCount(rootIndex) == 0;
}

bool NodeSelectorTreeViewWidgetRubbish::isNodeCompatibleWithModel(mega::MegaNode* node)
{
    return MegaSyncApp->getMegaApi()->isInRubbish(node);
}

void NodeSelectorTreeViewWidgetRubbish::makeViewConnections()
{
    connect(ui->tMegaFolders,
            &NodeSelectorTreeView::restoreClicked,
            mRestoreManager.get(),
            &RestoreNodeManager::onRestoreClicked);
}

QString NodeSelectorTreeViewWidgetRubbish::getRootText()
{
    return MegaNodeNames::getRubbishName();
}

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetRubbish::createModel()
{
    auto model = std::unique_ptr<NodeSelectorModelRubbish>(new NodeSelectorModelRubbish);

    mRestoreManager = std::make_shared<RestoreNodeManager>(model.get(), this);

    return model;
}

void NodeSelectorTreeViewWidgetRubbish::setViewPage()
{
    auto rootIndex = mModel->index(0, 0);
    if (mModel->rowCount(rootIndex) == 0 && showEmptyView())
    {
        ui->stackedWidget->setCurrentWidget(ui->emptyPage);

        // The rubbish has been emptied, so we can unset the loading view
        ui->tMegaFolders->loadingView().toggleLoadingScene(false);
    }
    else
    {
        ui->stackedWidget->setCurrentWidget(ui->treeViewPage);
    }
}

QIcon NodeSelectorTreeViewWidgetRubbish::getEmptyIcon()
{
    return Utilities::getIcon(QLatin1String("trash"),
                              Utilities::AttributeType::SMALL | Utilities::AttributeType::THIN |
                                  Utilities::AttributeType::OUTLINE);
}

NodeSelectorTreeViewWidget::EmptyLabelInfo NodeSelectorTreeViewWidgetRubbish::getEmptyLabel()
{
    EmptyLabelInfo info;
    info.description = tr("The Rubbish bin is empty");
    return info;
}

void NodeSelectorTreeViewWidgetRubbish::onRootIndexChanged(const QModelIndex& source_idx)
{
    Q_UNUSED(source_idx)
    ui->tMegaFolders->header()->hideSection(NodeSelectorModel::Column::USER);
    ui->tMegaFolders->header()->hideSection(NodeSelectorModel::Column::ACCESS);

    NodeSelectorTreeViewWidget::onRootIndexChanged(source_idx);
}
