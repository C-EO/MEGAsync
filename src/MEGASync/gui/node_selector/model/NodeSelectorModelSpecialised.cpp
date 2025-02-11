#include "NodeSelectorModelSpecialised.h"

#include "MegaApplication.h"
#include "MyBackupsHandle.h"
#include "Utilities.h"

#include <QApplication>
#include <QToolTip>

using namespace mega;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NodeSelectorModelCloudDrive::NodeSelectorModelCloudDrive(QObject *parent)
    : NodeSelectorModel(parent)
{
    setAcceptDragAndDrop(true);
}

void NodeSelectorModelCloudDrive::createRootNodes()
{    
    emit requestCloudDriveRootCreation();
}

int NodeSelectorModelCloudDrive::rootItemsCount() const
{
    return 1;
}

void NodeSelectorModelCloudDrive::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid())
    {
        fetchItemChildren(parent);
    }
}

void NodeSelectorModelCloudDrive::firstLoad()
{
    connect(this, &NodeSelectorModelCloudDrive::requestCloudDriveRootCreation, mNodeRequesterWorker, &NodeRequester::createCloudDriveRootItem);
    connect(mNodeRequesterWorker, &NodeRequester::megaCloudDriveRootItemCreated, this, &NodeSelectorModelCloudDrive::onRootItemCreated, Qt::QueuedConnection);

    addRootItems();
}

void NodeSelectorModelCloudDrive::onRootItemCreated()
{
    rootItemsLoaded();

    //Add the item of the Cloud Drive
    auto rootIndex(index(0,0));
    if(canFetchMore(rootIndex))
    {
        fetchItemChildren(rootIndex);
        mIndexesToBeExpanded.append(qMakePair(MegaSyncApp->getRootNode()->getHandle(), rootIndex));
        loadLevelFinished();
    }
    else
    {
        //In case the root item is empty (CD empty), let the model know that we have finished
        loadLevelFinished();
        sendBlockUiSignal(false);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
NodeSelectorModelIncomingShares::NodeSelectorModelIncomingShares(QObject *parent)
    : NodeSelectorModel(parent)
{
    MegaApi* megaApi = MegaSyncApp->getMegaApi();
    mSharedNodeList = std::unique_ptr<MegaNodeList>(megaApi->getInShares());

    setAcceptDragAndDrop(true);
}

void NodeSelectorModelIncomingShares::onItemInfoUpdated(int role)
{
    if(NodeSelectorModelItem* item = static_cast<NodeSelectorModelItem*>(sender()))
    {
        for(int i = 0; i < rowCount(); ++i)
        {
            QModelIndex idx = index(i, COLUMN::USER); //we only update this column because we retrieve the data in async mode
            if(idx.isValid())                         //so it is possible that we doesn´t have the information from the start
            {
                if(NodeSelectorModelItem* chkItem = static_cast<NodeSelectorModelItem*>(idx.internalPointer()))
                {
                    if(chkItem == item)
                    {
                        QVector<int> roles;
                        roles.append(role);
                        emit dataChanged(idx, idx, roles);
                        break;
                    }
                }
            }
        }
    }
}

void NodeSelectorModelIncomingShares::onRootItemAdded()
{
    NodeSelectorModel::onRootItemAdded();

    if (isMovingNodes())
    {
        moveProcessed();
    }
}

void NodeSelectorModelIncomingShares::onRootItemDeleted()
{
    NodeSelectorModel::onRootItemDeleted();

    if (isMovingNodes())
    {
        moveProcessed();
    }
}

bool NodeSelectorModelIncomingShares::rootNodeUpdated(mega::MegaNode* node)
{
    if(node->getChanges() & MegaNode::CHANGE_TYPE_INSHARE)
    {
        if(node->isInShare())
        {
            auto folderIndex = findIndexByNodeHandle(node->getHandle(), QModelIndex());
            if(!folderIndex.isValid())
            {
                auto totalRows = rowCount(QModelIndex());
                beginInsertRows(QModelIndex(), totalRows, totalRows);
                emit addIncomingSharesRoot(std::shared_ptr<mega::MegaNode>(node->copy()));
            }
            else
            {
                if(mNodeRequesterWorker->isIncomingShareCompatible(node))
                {
                    updateItemNode(folderIndex, std::shared_ptr<mega::MegaNode>(node->copy()));
                }
                else
                {
                    beginRemoveRows(QModelIndex(), folderIndex.row(), folderIndex.row());
                    emit deleteIncomingSharesRoot(std::shared_ptr<mega::MegaNode>(node->copy()));
                    return true;
                }
            }
        }

        return true;
    }
    else if(node->getParentHandle() == mega::INVALID_HANDLE && node->isFolder())
    {
        if(node->getChanges() & MegaNode::CHANGE_TYPE_REMOVED)
        {
            auto index = findIndexByNodeHandle(node->getHandle(), QModelIndex());
            if(index.isValid())
            {
                beginRemoveRows(QModelIndex(), index.row(), index.row());
                emit deleteIncomingSharesRoot(std::shared_ptr<mega::MegaNode>(node->copy()));
                return true;
            }
        }

        auto folderIndex = findIndexByNodeHandle(node->getHandle(), QModelIndex());
        if(folderIndex.isValid())
        {
            updateItemNode(folderIndex, std::shared_ptr<mega::MegaNode>(node->copy()));
            return true;
        }
    }

    return false;
}

bool NodeSelectorModelIncomingShares::canDropMimeData(const QMimeData* data,
                                                      Qt::DropAction action,
                                                      int,
                                                      int,
                                                      const QModelIndex& parent) const
{
    if (action == Qt::CopyAction || action == Qt::MoveAction)
    {
        if(parent.isValid())
        {
            auto item = getItemByIndex(parent);
            if(item)
            {
                auto node = item->getNode();
                if (node && node->isFolder())
                {
                    auto access = Utilities::getNodeAccess(node->getHandle());
                    if (access >= MegaShare::ACCESS_READWRITE)
                    {
                        if (action == Qt::CopyAction)
                        {
                            return true;
                        }
                        else
                        {
                            return checkDraggedMimeData(data);
                        }
                    }
                }
            }
        }
    }

    return false;
}

QModelIndex NodeSelectorModelIncomingShares::rootIndex(const QModelIndex& visualRootIndex) const
{
    return visualRootIndex;
}

bool NodeSelectorModelIncomingShares::canBeDeleted() const
{
    return true;
}

void NodeSelectorModelIncomingShares::setCurrentRootIndex(const QModelIndex& rootIndex)
{
    if (rootIndex.isValid())
    {
        NodeSelectorModel::setCurrentRootIndex(rootIndex);
    }
}

void NodeSelectorModelIncomingShares::onRootItemsCreated()
{
    rootItemsLoaded();

    if(!mNodesToLoad.isEmpty())
    {
        auto index = getIndexFromNode(mNodesToLoad.last(), QModelIndex());
        if(canFetchMore(index))
        {
            fetchMore(index);
        }
    }
    else
    {
        loadLevelFinished();
    }
}

void NodeSelectorModelIncomingShares::createRootNodes()
{
    emit requestIncomingSharesRootCreation(mSharedNodeList);
}

int NodeSelectorModelIncomingShares::rootItemsCount() const
{
    return mSharedNodeList->size();
}

void NodeSelectorModelIncomingShares::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid())
    {
        fetchItemChildren(parent);
    }
}

void NodeSelectorModelIncomingShares::firstLoad()
{
    connect(this, &NodeSelectorModelIncomingShares::requestIncomingSharesRootCreation, mNodeRequesterWorker, &NodeRequester::createIncomingSharesRootItems);
    connect(this, &NodeSelectorModelIncomingShares::addIncomingSharesRoot, mNodeRequesterWorker, &NodeRequester::addIncomingSharesRootItem);
    connect(this, &NodeSelectorModelIncomingShares::deleteIncomingSharesRoot, this, [this](std::shared_ptr<mega::MegaNode> node)
            {
                mNodeRequesterWorker->removeRootItem(node);
            });
    connect(mNodeRequesterWorker, &NodeRequester::megaIncomingSharesRootItemsCreated, this, &NodeSelectorModelIncomingShares::onRootItemsCreated, Qt::QueuedConnection);

    addRootItems();
}

///////////////////////////////////////////////////////////////////////////////////////////////
NodeSelectorModelBackups::NodeSelectorModelBackups(QObject *parent)
    : NodeSelectorModel(parent)
    , mBackupDevicesSize(0)
{
}

NodeSelectorModelBackups::~NodeSelectorModelBackups()
{
}

void NodeSelectorModelBackups::createRootNodes()
{
    emit requestBackupsRootCreation(mBackupsHandle);
}

int NodeSelectorModelBackups::rootItemsCount() const
{
    return 1;
}

void NodeSelectorModelBackups::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid())
    {
        fetchItemChildren(parent);
    }
}

void NodeSelectorModelBackups::firstLoad()
{
    connect(this, &NodeSelectorModelBackups::requestBackupsRootCreation, mNodeRequesterWorker, &NodeRequester::createBackupRootItems);
    connect(mNodeRequesterWorker, &NodeRequester::megaBackupRootItemsCreated, this, &NodeSelectorModelBackups::onRootItemCreated, Qt::QueuedConnection);

    auto backupsRequest = UserAttributes::MyBackupsHandle::requestMyBackupsHandle();
    mBackupsHandle = backupsRequest->getMyBackupsHandle();
    connect(backupsRequest.get(), &UserAttributes::MyBackupsHandle::attributeReady,
            this, &NodeSelectorModelBackups::onMyBackupsHandleReceived);

    if(backupsRequest->isAttributeReady())
    {
        onMyBackupsHandleReceived(backupsRequest->getMyBackupsHandle());
    }
    else
    {
        addRootItems();
    }
}

bool NodeSelectorModelBackups::canBeDeleted() const
{
    return false;
}

bool NodeSelectorModelBackups::canDropMimeData(const QMimeData*,
                                               Qt::DropAction,
                                               int,
                                               int,
                                               const QModelIndex&) const
{
    return false;
}

void NodeSelectorModelBackups::onMyBackupsHandleReceived(mega::MegaHandle handle)
{
    mBackupsHandle = handle;
    if (mBackupsHandle != INVALID_HANDLE)
    {
        addRootItems();
    }
}

bool NodeSelectorModelBackups::addToLoadingList(const std::shared_ptr<MegaNode> node)
{
    return node && node->getType() != mega::MegaNode::TYPE_VAULT;
}

void NodeSelectorModelBackups::loadLevelFinished()
{
    if (mIndexesToBeExpanded.size() == 1 && mIndexesToBeExpanded.at(0).second == index(0, 0))
    {
        QModelIndex rootIndex(index(0, 0));
        int rowcount = rowCount(rootIndex);
        for(int i = 0 ; i < rowcount; i++)
        {
            auto idx = index(i, 0, rootIndex);
            if(canFetchMore(idx))
            {
                mBackupDevicesSize++;
                fetchItemChildren(idx);
            }
        }
    }

    if(mBackupDevicesSize > 0)
    {
        mBackupDevicesSize--;
    }
    if(mBackupDevicesSize == 0)
    {
        NodeSelectorModel::loadLevelFinished();
    }
}

void NodeSelectorModelBackups::onRootItemCreated()
{
    rootItemsLoaded();

    QModelIndex rootIndex(index(0, 0));
    //Add the item of the Backups Drive
    if(canFetchMore(rootIndex))
    {
        auto backupItem(getItemByIndex(rootIndex));
        if (backupItem)
        {
            mIndexesToBeExpanded.append(qMakePair(backupItem->getNode()->getHandle(), rootIndex));
        }
        fetchItemChildren(rootIndex);
    }
    else
    {
        loadLevelFinished();
    }
}

NodeSelectorModelSearch::NodeSelectorModelSearch(NodeSelectorModelItemSearch::Types allowedTypes, QObject *parent)
    : NodeSelectorModel(parent),
      mAllowedTypes(allowedTypes)
{
    qRegisterMetaType<NodeSelectorModelItemSearch::Types>("NodeSelectorModelItemSearch::Types");
}

NodeSelectorModelSearch::~NodeSelectorModelSearch()
{

}

void NodeSelectorModelSearch::firstLoad()
{
    connect(this, &NodeSelectorModelSearch::searchNodes, mNodeRequesterWorker, &NodeRequester::search);
    connect(this, &NodeSelectorModelSearch::requestAddSearchRootItem, mNodeRequesterWorker, &NodeRequester::addSearchRootItem);
    connect(this, &NodeSelectorModelSearch::requestDeleteSearchRootItem, this, [this](std::shared_ptr<mega::MegaNode> node)
            {
                mNodeRequesterWorker->removeRootItem(node);
            });
    connect(mNodeRequesterWorker, &NodeRequester::searchItemsCreated, this, &NodeSelectorModelSearch::onRootItemsCreated, Qt::QueuedConnection);
}

void NodeSelectorModelSearch::createRootNodes()
{
    //pure virtual function in the base class, in first stage this model is empty so not need to put any code here.
}

void NodeSelectorModelSearch::searchByText(const QString &text)
{
    mNodeRequesterWorker->restartSearch();
    addRootItems();
    emit searchNodes(text, mAllowedTypes);
}

void NodeSelectorModelSearch::stopSearch()
{
    mNodeRequesterWorker->restartSearch();
}

int NodeSelectorModelSearch::rootItemsCount() const
{
    return 0;
}

bool NodeSelectorModelSearch::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return false;
}

QVariant NodeSelectorModelSearch::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
    {
        return QVariant();
    }
    if(index.column() == NODE)
    {
        switch(role)
        {
        case toInt(NodeRowDelegateRoles::INDENT_ROLE):
        {
            return -15;
        }
        case toInt(NodeRowDelegateRoles::SMALL_ICON_ROLE):
        {
            return false;
        }
        }
    }
    else if(index.column() == STATUS && role == Qt::DecorationRole)
    {
        NodeSelectorModelItem* item = static_cast<NodeSelectorModelItem*>(index.internalPointer());
        if(item->getStatus() == NodeSelectorModelItem::Status::SYNC_CHILD)
        {
            QIcon statusIcons; //first is selected state icon / second is normal state icon
            statusIcons.addFile(QLatin1String("://images/Item-sync-press.png"), QSize(), QIcon::Selected); //selected style icon
            statusIcons.addFile(QLatin1String("://images/Item-sync-rest.png"), QSize(), QIcon::Normal); //normal style icon
            return statusIcons;
        }
    }
    return NodeSelectorModel::data(index, role);
}

void NodeSelectorModelSearch::addNodes(QList<std::shared_ptr<mega::MegaNode>> nodes, const QModelIndex &parent)
{
    auto totalRows = rowCount(parent);
    beginInsertRows(QModelIndex(), totalRows, totalRows + nodes.size() - 1);
    emit requestAddSearchRootItem(nodes, mAllowedTypes);
}

bool NodeSelectorModelSearch::rootNodeUpdated(mega::MegaNode* node)
{
    if (node->getChanges() & MegaNode::CHANGE_TYPE_INSHARE)
    {
        if (node->isInShare())
        {
            auto totalRows = rowCount(QModelIndex());
            beginInsertRows(QModelIndex(), totalRows, totalRows);
            QList<std::shared_ptr<mega::MegaNode>> nodes;
            emit requestAddSearchRootItem(nodes << std::shared_ptr<mega::MegaNode>(node->copy()),
                                          mAllowedTypes);
        }

        return true;
    }
    else if (node->getChanges() & (MegaNode::CHANGE_TYPE_PARENT))
    {
        auto index = findIndexByNodeHandle(node->getHandle(), QModelIndex());
        if (index.isValid())
        {
            auto item = getItemByIndex(index);
            if (auto searchItem = dynamic_cast<NodeSelectorModelItemSearch*>(item))
            {
                auto newType(calculateSearchType(node));
                auto oldType(searchItem->getType());
                if (newType != oldType)
                {
                    searchItem->setType(newType);
                    updateRow(index);
                    emit nodeTypeHasChanged();
                    return true;
                }
            }
        }
    }
    else if (node->getChanges() & MegaNode::CHANGE_TYPE_REMOVED)
    {
        auto index = findIndexByNodeHandle(node->getHandle(), QModelIndex());
        if (index.isValid())
        {
            beginRemoveRows(QModelIndex(), index.row(), index.row());
            emit requestDeleteSearchRootItem(std::shared_ptr<mega::MegaNode>(node->copy()));
            return true;
        }
    }

    auto folderIndex = findIndexByNodeHandle(node->getHandle(), QModelIndex());
    if (folderIndex.isValid())
    {
        updateItemNode(folderIndex, std::shared_ptr<mega::MegaNode>(node->copy()));
        return true;
    }

    return false;
}

bool NodeSelectorModelSearch::canDropMimeData(const QMimeData*,
                                              Qt::DropAction,
                                              int,
                                              int,
                                              const QModelIndex&) const
{
    return false;
}

bool NodeSelectorModelSearch::canCopyNodes() const
{
    return false;
}

void NodeSelectorModelSearch::proxyInvalidateFinished()
{
    mNodeRequesterWorker->lockSearchMutex(false);
}

void NodeSelectorModelSearch::onRootItemAdded()
{
    NodeSelectorModel::onRootItemAdded();

    if (isMovingNodes())
    {
        moveProcessed();
    }
}

void NodeSelectorModelSearch::onRootItemDeleted()
{
    NodeSelectorModel::onRootItemDeleted();

    if (isMovingNodes())
    {
        moveProcessed();
    }
}

void NodeSelectorModelSearch::onRootItemsCreated()
{
    if(mNodeRequesterWorker->trySearchLock())
    {
        rootItemsLoaded();
        emit levelsAdded(mIndexesToBeExpanded, true);
    }
}

const NodeSelectorModelItemSearch::Types &NodeSelectorModelSearch::searchedTypes() const
{
    return mNodeRequesterWorker->searchedTypes();
}

NodeSelectorModelItemSearch::Types
    NodeSelectorModelSearch::calculateSearchType(mega::MegaNode* node)
{
    NodeSelectorModelItemSearch::Types type;

    if (MegaSyncApp->getMegaApi()->isInCloud(node))
    {
        type = NodeSelectorModelItemSearch::Type::CLOUD_DRIVE;
    }
    else if (MegaSyncApp->getMegaApi()->isInVault(node))
    {
        type = NodeSelectorModelItemSearch::Type::BACKUP;
    }
    else if (MegaSyncApp->getMegaApi()->isInRubbish(node))
    {
        type = NodeSelectorModelItemSearch::Type::RUBBISH;
    }
    else
    {
        type = NodeSelectorModelItemSearch::Type::INCOMING_SHARE;
    }

    return type;
}

///////////////////////////////////////////////////////////////////////////////////////////
NodeSelectorModelRubbish::NodeSelectorModelRubbish(QObject *parent)
    : NodeSelectorModel(parent)
{
    setAcceptDragAndDrop(true);
}

void NodeSelectorModelRubbish::onItemInfoUpdated(int role)
{
    if(NodeSelectorModelItem* item = static_cast<NodeSelectorModelItem*>(sender()))
    {
        for(int i = 0; i < rowCount(); ++i)
        {
            QModelIndex idx = index(i, COLUMN::USER); //we only update this column because we retrieve the data in async mode
            if(idx.isValid())                         //so it is possible that we doesn´t have the information from the start
            {
                if(NodeSelectorModelItem* chkItem = static_cast<NodeSelectorModelItem*>(idx.internalPointer()))
                {
                    if(chkItem == item)
                    {
                        QVector<int> roles;
                        roles.append(role);
                        emit dataChanged(idx, idx, roles);
                        break;
                    }
                }
            }
        }
    }
}

void NodeSelectorModelRubbish::onRootItemsCreated()
{
    rootItemsLoaded();

    //Add the item of the Cloud Drive
    auto rootIndex(index(0,0));
    if(canFetchMore(rootIndex))
    {
        fetchItemChildren(rootIndex);
        auto rubbishItem(getItemByIndex(rootIndex));
        if (rubbishItem)
        {
            mIndexesToBeExpanded.append(qMakePair(rubbishItem->getNode()->getHandle(), rootIndex));
        }
    }
    else
    {
        //In case the root item is empty (CD empty), let the model know that we have finished
        loadLevelFinished();
        sendBlockUiSignal(false);
    }
}

void NodeSelectorModelRubbish::createRootNodes()
{
    emit requestRubbishRootCreation();
}

int NodeSelectorModelRubbish::rootItemsCount() const
{
    return 1;
}

void NodeSelectorModelRubbish::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid())
    {
        fetchItemChildren(parent);
    }
}

void NodeSelectorModelRubbish::firstLoad()
{
    connect(this,
            &NodeSelectorModelRubbish::requestRubbishRootCreation,
            mNodeRequesterWorker,
            &NodeRequester::createRubbishRootItems);
    connect(this, &NodeSelectorModelRubbish::deleteRubbishRoot, this, [this](std::shared_ptr<mega::MegaNode> node)
            {
                mNodeRequesterWorker->removeRootItem(node);
            });
    connect(mNodeRequesterWorker, &NodeRequester::megaRubbishRootItemsCreated, this, &NodeSelectorModelRubbish::onRootItemsCreated, Qt::QueuedConnection);

    addRootItems();
}

bool NodeSelectorModelRubbish::isNodeAccepted(MegaNode* node)
{
    return MegaSyncApp->getMegaApi()->isInRubbish(node);
}

bool NodeSelectorModelRubbish::canDropMimeData(const QMimeData* data,
                                               Qt::DropAction action,
                                               int,
                                               int,
                                               const QModelIndex& parent) const
{
    if (action == Qt::CopyAction || action == Qt::MoveAction)
    {
        if (parent.isValid())
        {
            auto item = getItemByIndex(parent);
            if (item)
            {
                auto node = item->getNode();
                if (node && node->isFolder())
                {
                    auto access = Utilities::getNodeAccess(node->getHandle());
                    if (access == MegaShare::ACCESS_OWNER)
                    {
                        if (action == Qt::CopyAction)
                        {
                            return true;
                        }
                        else
                        {
                            return checkDraggedMimeData(data);
                        }
                    }
                }
            }
        }
    }

    return false;
}

bool NodeSelectorModelRubbish::canCopyNodes() const
{
    return false;
}
