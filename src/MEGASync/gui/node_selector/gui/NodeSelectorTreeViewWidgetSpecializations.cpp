#include "NodeSelectorTreeViewWidgetSpecializations.h"
#include "ui_NodeSelectorTreeViewWidget.h"
#include "../model/NodeSelectorProxyModel.h"
#include "../model/NodeSelectorModel.h"
#include "../model/NodeSelectorModelSpecialised.h"
#include <MegaApplication.h>

#include "MegaNodeNames.h"

///////////////////////////////////////////////////////////////////
NodeSelectorTreeViewWidgetCloudDrive::NodeSelectorTreeViewWidgetCloudDrive(SelectTypeSPtr mode, QWidget *parent)
    : NodeSelectorTreeViewWidget(mode, parent)
{
    setTitle(MegaNodeNames::getCloudDriveName());
    ui->searchEmptyInfoWidget->hide();
}

void NodeSelectorTreeViewWidgetCloudDrive::itemsRestored(mega::MegaHandle& handle, bool parentLoaded)
{
    this->setAcceptDrops(true);
    ui->tMegaFolders->setAcceptDrops(true);
    if(!parentLoaded)
    {
        setSelectedNodeHandle(handle);
    }
    else
    {
        mRestoredHandle = handle;
    }
}

void NodeSelectorTreeViewWidgetCloudDrive::setShowEmptyView(bool newShowEmptyView)
{
    mShowEmptyView = newShowEmptyView;
}

void NodeSelectorTreeViewWidgetCloudDrive::onRowsInserted()
{
    NodeSelectorTreeViewWidget::onRowsInserted();

    if(mRestoredHandle != mega::INVALID_HANDLE)
    {
        setSelectedNodeHandle(mRestoredHandle);
        mRestoredHandle = mega::INVALID_HANDLE;
    }
}

QString NodeSelectorTreeViewWidgetCloudDrive::getRootText()
{
    return MegaNodeNames::getCloudDriveName();
}

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetCloudDrive::createModel()
{
    return std::unique_ptr<NodeSelectorModelCloudDrive>(new NodeSelectorModelCloudDrive);
}

void NodeSelectorTreeViewWidgetCloudDrive::modelLoaded()
{
    auto rootIndex = mModel->index(0,0);
    if(mModel->rowCount(rootIndex) == 0 && showEmptyView())
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
    return QIcon(QString::fromUtf8("://images/node_selector/view/cloud.png"));
}

bool NodeSelectorTreeViewWidgetCloudDrive::isCurrentRootIndexReadOnly()
{
    return false;
}

void NodeSelectorTreeViewWidgetCloudDrive::onRootIndexChanged(const QModelIndex &source_idx)
{
    Q_UNUSED(source_idx)
    ui->tMegaFolders->header()->hideSection(NodeSelectorModel::COLUMN::USER);
}

/////////////////////////////////////////////////////////////////
NodeSelectorTreeViewWidgetIncomingShares::NodeSelectorTreeViewWidgetIncomingShares(SelectTypeSPtr mode, QWidget *parent)
    : NodeSelectorTreeViewWidget(mode, parent)
{
    setTitle(MegaNodeNames::getIncomingSharesName());
    ui->searchEmptyInfoWidget->hide();
}

QString NodeSelectorTreeViewWidgetIncomingShares::getRootText()
{
    return MegaNodeNames::getIncomingSharesName();
}

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetIncomingShares::createModel()
{
    return std::unique_ptr<NodeSelectorModelIncomingShares>(new NodeSelectorModelIncomingShares);
}

void NodeSelectorTreeViewWidgetIncomingShares::onRootIndexChanged(const QModelIndex &idx)
{
    if(idx.isValid())
    {
        QModelIndex in_share_idx = getParentIncomingShareByIndex(idx);
        in_share_idx = in_share_idx.sibling(in_share_idx.row(), NodeSelectorModel::COLUMN::USER);
        QPixmap pm = qvariant_cast<QPixmap>(in_share_idx.data(Qt::DecorationRole));
        QString tooltip = in_share_idx.data(Qt::ToolTipRole).toString();
        ui->lOwnerIcon->setToolTip(tooltip);
        ui->lOwnerIcon->setPixmap(pm);
        ui->avatarSpacer->spacerItem()->changeSize(10, 0);
        ui->tMegaFolders->header()->hideSection(NodeSelectorModel::COLUMN::USER);
    }
    else
    {
        ui->tMegaFolders->header()->showSection(NodeSelectorModel::COLUMN::USER);
    }
}

bool NodeSelectorTreeViewWidgetIncomingShares::isCurrentRootIndexReadOnly()
{
    auto rootIndex(ui->tMegaFolders->rootIndex());
    if(rootIndex.isValid())
    {
        auto rootNode = mProxyModel->getNode(rootIndex);
        if(rootNode)
        {
            return MegaSyncApp->getMegaApi()->getAccess(rootNode.get()) <= mega::MegaShare::ACCESS_READ;
        }
    }

    return true;
}

bool NodeSelectorTreeViewWidgetIncomingShares::isCurrentSelectionReadOnly()
{
    auto selectedRows = ui->tMegaFolders->selectionModel()->selectedRows();
    auto anyReadOnly(selectedRows.isEmpty() ? isCurrentRootIndexReadOnly() : false);
    foreach(auto index, selectedRows)
    {
        auto rootIndex(getRootIndexFromIndex(index));
        if(rootIndex.isValid())
        {
            auto rootNode = mProxyModel->getNode(rootIndex);
            if(rootNode)
            {
                if(MegaSyncApp->getMegaApi()->getAccess(rootNode.get()) <= mega::MegaShare::ACCESS_READ)
                {
                    anyReadOnly = true;
                    break;
                }
            }
        }
    }

    return anyReadOnly;
}

QIcon NodeSelectorTreeViewWidgetIncomingShares::getEmptyIcon()
{
    return QIcon(QString::fromUtf8("://images/node_selector/view/folder_share.png"));
}

/////////////////////////////////////////////////////////////////
NodeSelectorTreeViewWidgetBackups::NodeSelectorTreeViewWidgetBackups(SelectTypeSPtr mode, QWidget *parent)
    : NodeSelectorTreeViewWidget(mode, parent)
{
    setTitle(MegaNodeNames::getBackupsName());
    ui->searchEmptyInfoWidget->hide();
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
    return QIcon(QString::fromUtf8("://images/node_selector/view/database.png"));
}

void NodeSelectorTreeViewWidgetBackups::onRootIndexChanged(const QModelIndex &idx)
{
    Q_UNUSED(idx)
    ui->tMegaFolders->header()->hideSection(NodeSelectorModel::COLUMN::USER);
}
/////////////////////////////////////////////////////////////////

NodeSelectorTreeViewWidgetSearch::NodeSelectorTreeViewWidgetSearch(SelectTypeSPtr mode, QWidget *parent)
    : NodeSelectorTreeViewWidget(mode, parent)
    , mHasRows(false)

{
    setTitleText(tr("Searching:"));
    ui->bBack->hide();
    ui->bForward->hide();
    connect(ui->cloudDriveSearch, &QToolButton::clicked, this, &NodeSelectorTreeViewWidgetSearch::onCloudDriveSearchClicked);
    connect(ui->incomingSharesSearch, &QToolButton::clicked, this, &NodeSelectorTreeViewWidgetSearch::onIncomingSharesSearchClicked);
    connect(ui->backupsSearch, &QToolButton::clicked, this, &NodeSelectorTreeViewWidgetSearch::onBackupsSearchClicked);
}

void NodeSelectorTreeViewWidgetSearch::search(const QString &text)
{
    changeButtonsWidgetSizePolicy(true);
    ui->stackedWidget->setCurrentWidget(ui->treeViewPage);
    ui->searchButtonsWidget->setVisible(false);
    auto search_model = static_cast<NodeSelectorModelSearch*>(mModel.get());
    search_model->searchByText(text);
    ui->searchingText->setText(text);
    ui->searchNotFoundText->setText(text);
    ui->searchingText->setVisible(true);
}

void NodeSelectorTreeViewWidgetSearch::stopSearch()
{
    auto search_model = static_cast<NodeSelectorModelSearch*>(mModel.get());
    search_model->stopSearch();
    mHasRows = false;
}

std::unique_ptr<NodeSelectorProxyModel> NodeSelectorTreeViewWidgetSearch::createProxyModel()
{
    auto proxy =  std::unique_ptr<NodeSelectorProxyModelSearch>(new NodeSelectorProxyModelSearch);
    //The search view is the only one with a real proxy model (in terms on filterAcceptsRow)
    connect(proxy.get(), &QAbstractItemModel::rowsInserted, this, &NodeSelectorTreeViewWidget::onRowsInserted);
    connect(proxy.get(), &QAbstractItemModel::rowsRemoved, this, &NodeSelectorTreeViewWidget::onRowsRemoved);
    return proxy;
}

bool NodeSelectorTreeViewWidgetSearch::newNodeCanBeAdded(mega::MegaNode *node)
{
    auto nodeName(QString::fromUtf8(node->getName()));
    auto containsText = nodeName.contains(ui->searchingText->text(),Qt::CaseInsensitive);
    return containsText;
}

QModelIndex NodeSelectorTreeViewWidgetSearch::getAddedNodeParent(mega::MegaHandle parentHandle)
{
    Q_UNUSED(parentHandle)
    return QModelIndex();
}

bool NodeSelectorTreeViewWidgetSearch::containsIndexToAddOrUpdate(mega::MegaNode* node, const mega::MegaHandle&)
{
    if(mHasRows && node)
    {
        auto index = mModel->findItemByNodeHandle(node->getHandle(), QModelIndex());
        if(index.isValid())
        {
            return true;
        }
        else
        {
            return newNodeCanBeAdded(node);
        }

    }

    return false;
}

void NodeSelectorTreeViewWidgetSearch::onBackupsSearchClicked()
{
    auto proxy_model = static_cast<NodeSelectorProxyModelSearch*>(mProxyModel.get());
    proxy_model->setMode(NodeSelectorModelItemSearch::Type::BACKUP);
    ui->tMegaFolders->setColumnHidden(NodeSelectorModel::USER, true);
}

void NodeSelectorTreeViewWidgetSearch::onIncomingSharesSearchClicked()
{
    auto proxy_model = static_cast<NodeSelectorProxyModelSearch*>(mProxyModel.get());
    proxy_model->setMode(NodeSelectorModelItemSearch::Type::INCOMING_SHARE);
    ui->tMegaFolders->setColumnHidden(NodeSelectorModel::USER, false);
}

void NodeSelectorTreeViewWidgetSearch::onCloudDriveSearchClicked()
{
    auto proxy_model = static_cast<NodeSelectorProxyModelSearch*>(mProxyModel.get());
    proxy_model->setMode(NodeSelectorModelItemSearch::Type::CLOUD_DRIVE);
    ui->tMegaFolders->setColumnHidden(NodeSelectorModel::USER, true);
}

void NodeSelectorTreeViewWidgetSearch::onItemDoubleClick(const QModelIndex &index)
{
    auto node = qvariant_cast<std::shared_ptr<MegaNode>>(index.data(toInt(NodeSelectorModelRoles::NODE_ROLE)));
    emit nodeDoubleClicked(node, true);
}

void NodeSelectorTreeViewWidgetSearch::changeButtonsWidgetSizePolicy(bool state)
{
    auto buttonWidgetSizePolicy = ui->searchButtonsWidget->sizePolicy();
    buttonWidgetSizePolicy.setRetainSizeWhenHidden(state);
    ui->searchButtonsWidget->setSizePolicy(buttonWidgetSizePolicy);
}

QString NodeSelectorTreeViewWidgetSearch::getRootText()
{
    return tr("Searching:");
}

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetSearch::createModel()
{
    return std::unique_ptr<NodeSelectorModelSearch>(new NodeSelectorModelSearch(getSelectType()->allowedTypes()));
}

QIcon NodeSelectorTreeViewWidgetSearch::getEmptyIcon()
{
    return QIcon(QString::fromUtf8("://images/node_selector/view/search.png"));
}

void NodeSelectorTreeViewWidgetSearch::modelLoaded()
{
    if(!mModel)
    {
        return;
    }

    changeButtonsWidgetSizePolicy(false);
    NodeSelectorTreeViewWidget::modelLoaded();

    NodeSelectorModelItemSearch::Types searchedTypes = NodeSelectorModelItemSearch::Type::NONE;
    auto searchModel = dynamic_cast<NodeSelectorModelSearch*>(mModel.get());
    if(searchModel)
    {
        searchedTypes = searchModel->searchedTypes();
    }

    ui->backupsSearch->setVisible(searchedTypes.testFlag(NodeSelectorModelItemSearch::Type::BACKUP));
    ui->incomingSharesSearch->setVisible(searchedTypes.testFlag(NodeSelectorModelItemSearch::Type::INCOMING_SHARE));
    ui->cloudDriveSearch->setVisible(searchedTypes.testFlag(NodeSelectorModelItemSearch::Type::CLOUD_DRIVE));
    ui->searchButtonsWidget->setVisible(true);

    QToolButton* buttonToCheck(nullptr);

    auto buttons = ui->searchButtonsWidget->findChildren<QToolButton*>();
    foreach(auto& button, buttons)
    {
        if(button->isVisible() && button->isChecked())
        {
            buttonToCheck = button;
            break;
        }
        else if(!buttonToCheck && button->isVisible())
        {
            buttonToCheck = button;
        }
    }

    checkAndClick(buttonToCheck);

    if(ui->tMegaFolders->model())
    {
        mHasRows = ui->tMegaFolders->model()->rowCount() > 0;
        if(!mHasRows && showEmptyView())
        {
            ui->stackedWidget->setCurrentWidget(ui->emptyPage);
            return;
        }
    }
    ui->stackedWidget->setCurrentWidget(ui->treeViewPage);
}

void NodeSelectorTreeViewWidgetSearch::checkAndClick(QToolButton* button)
{
    if(button  && button->isVisible())
    {
        button->setChecked(true);
        emit button->clicked(true);
    }
}

///////////////////////
NodeSelectorTreeViewWidgetRubbish::NodeSelectorTreeViewWidgetRubbish(SelectTypeSPtr mode, QWidget *parent)
    : NodeSelectorTreeViewWidget(mode, parent)
{
    setTitle(MegaNodeNames::getRubbishName());
    ui->searchEmptyInfoWidget->hide();
}

void NodeSelectorTreeViewWidgetRubbish::setShowEmptyView(bool newShowEmptyView)
{
    mShowEmptyView = newShowEmptyView;
}

bool NodeSelectorTreeViewWidgetRubbish::isEmpty() const
{
    auto rootIndex = mModel->index(0,0);
    return mModel->rowCount(rootIndex) == 0;
}

void NodeSelectorTreeViewWidgetRubbish::restoreItems(const QList<mega::MegaHandle> &handles, bool parentLoaded, mega::MegaHandle firstRestoredHandle)
{
    mRestoredItems = handles;
    mFirstRestoredHandle = firstRestoredHandle;
    mFirstRestoredHandleParentLoaded = parentLoaded;

    foreach(auto handle, handles)
    {
        auto node = std::shared_ptr<MegaNode>(mMegaApi->getNodeByHandle(handle));
        if (node)
        {
            auto newParent = std::unique_ptr<MegaNode>(mMegaApi->getNodeByHandle(node->getRestoreHandle()));
            mMegaApi->moveNode(node.get(),
                               newParent.get(),
                               new mega::OnFinishOneShot(MegaSyncApp->getMegaApi(), this,
                                                         [this, handle]
                                                         (bool isContextValid, const mega::MegaRequest& request, const mega::MegaError& e)
                                                         {
                                   if(isContextValid &&
                                       e.getErrorCode() == mega::MegaError::API_OK)
                                   {
                                       mRestoredItems.removeOne(handle);
                                       if(mRestoredItems.isEmpty())
                                       {
                                           emit itemsRestored(mFirstRestoredHandle, mFirstRestoredHandleParentLoaded);
                                       }
                                   }
                               }));
        }
    }
}

void NodeSelectorTreeViewWidgetRubbish::makeCustomConnections()
{
    connect(ui->tMegaFolders, &NodeSelectorTreeView::restoreClicked, this, &NodeSelectorTreeViewWidgetRubbish::onRestoreClicked);
}

void NodeSelectorTreeViewWidgetRubbish::onRestoreClicked(const QList<mega::MegaHandle>& handles)
{
    emit itemsRestoreRequested(handles);
}

QString NodeSelectorTreeViewWidgetRubbish::getRootText()
{
    return MegaNodeNames::getRubbishName();
}

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetRubbish::createModel()
{
    return std::unique_ptr<NodeSelectorModelRubbish>(new NodeSelectorModelRubbish);
}

void NodeSelectorTreeViewWidgetRubbish::modelLoaded()
{
    auto rootIndex = mModel->index(0,0);
    if(mModel->rowCount(rootIndex) == 0 && showEmptyView())
    {
        ui->stackedWidget->setCurrentWidget(ui->emptyPage);
    }
    else
    {
        ui->stackedWidget->setCurrentWidget(ui->treeViewPage);
    }
}

QIcon NodeSelectorTreeViewWidgetRubbish::getEmptyIcon()
{
    return QIcon(QString::fromUtf8("://images/node_selector/view/rubbish.png"));
}

void NodeSelectorTreeViewWidgetRubbish::onRootIndexChanged(const QModelIndex &source_idx)
{
    Q_UNUSED(source_idx)
    ui->tMegaFolders->header()->hideSection(NodeSelectorModel::COLUMN::USER);
}
