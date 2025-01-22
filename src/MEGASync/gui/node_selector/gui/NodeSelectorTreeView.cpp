#include "NodeSelectorTreeView.h"

#include "MegaApplication.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorModelItem.h"
#include "NodeSelectorProxyModel.h"
#include "Platform.h"
#include "CreateRemoveSyncsManager.h"

#include <QMenu>
#include <QMouseEvent>
#include <QPainter>

NodeSelectorTreeView::NodeSelectorTreeView(QWidget* parent) :
    LoadingSceneView<NodeSelectorLoadingDelegate, QTreeView>(parent),
    mMegaApi(MegaSyncApp->getMegaApi())
{
    installEventFilter(this);
    loadingView().setDelayTimeToShowInMs(150);
}

QModelIndex NodeSelectorTreeView::getIndexFromSourceModel(const QModelIndex& index) const
{
    if(!index.isValid())
    {
        return QModelIndex();
    }
    return proxyModel()->getIndexFromSource(index);
}

NodeSelectorProxyModel *NodeSelectorTreeView::proxyModel() const
{
    return static_cast<NodeSelectorProxyModel*>(model());
}

//Only used for single selection mode
MegaHandle NodeSelectorTreeView::getSelectedNodeHandle()
{
    MegaHandle ret = INVALID_HANDLE;
    if(!selectionModel())
    {
        return ret;
    }

    if(selectionModel()->selectedRows().size() == 1)
    {
        if(auto node = proxyModel()->getNode(selectionModel()->selectedRows().first()))
            ret = node->getHandle();
    }
    return ret;
}

QList<MegaHandle> NodeSelectorTreeView::getMultiSelectionNodeHandle() const
{
    QList<MegaHandle> ret;

    if(!selectionModel())
    {
        return ret;
    }

    auto selectedRows = selectionModel()->selectedRows();

    //If there is no selection, add the root index
    if(selectedRows.isEmpty())
    {
        auto index(rootIndex());
        if(index.isValid())
        {
            auto item = proxyModel()->getMegaModel()->getItemByIndex(index);
            if(item)
            {
                ret.append(item->getNode()->getHandle());
            }
        }
    }
    else
    {
        foreach(auto& s_index, selectedRows)
        {
            if(auto node = proxyModel()->getNode(s_index))
                ret.append(node->getHandle());
        }
    }

    return ret;
}

void NodeSelectorTreeView::setModel(QAbstractItemModel *model)
{
    QTreeView::setModel(model);
    connect(proxyModel(), &NodeSelectorProxyModel::navigateReady, this, &NodeSelectorTreeView::onNavigateReady);
}

void NodeSelectorTreeView::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    auto item = qvariant_cast<NodeSelectorModelItem*>(index.data(toInt(NodeSelectorModelRoles::MODEL_ITEM_ROLE)));
    if(item && (item->isCloudDrive() || item->isVault() || item->isRubbishBin()))
    {
        QStyleOptionViewItem opt = viewOptions();
        opt.rect = rect;
        if(!selectionModel())
        {
            return;
        }
        if(selectionModel()->isSelected(index))
        {
            opt.state |= QStyle::State_Selected;
        }
        style()->drawPrimitive(QStyle::PE_IndicatorBranch, &opt, painter, this);
        return;
    }
    QTreeView::drawBranches(painter, rect, index);
}

void NodeSelectorTreeView::mousePressEvent(QMouseEvent *event)
{
    bool accept = true;
    if (style()->styleHint(QStyle::SH_ListViewExpand_SelectMouseType, 0, this) == QEvent::MouseButtonPress)
    {
        accept = mousePressorReleaseEvent(event);
    }

    if(accept)
    {
        QTreeView::mousePressEvent(event);
    }
}


void NodeSelectorTreeView::mouseReleaseEvent(QMouseEvent *event)
{
    bool accept = true;
    if (style()->styleHint(QStyle::SH_ListViewExpand_SelectMouseType, 0, this) == QEvent::MouseButtonRelease)
    {
        accept = mousePressorReleaseEvent(event);
    }

    if(accept)
    {
        QTreeView::mouseReleaseEvent(event);
    }
}

void NodeSelectorTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() != Qt::RightButton)
    {
        QModelIndex clickedIndex = indexAt(event->pos());
        if(clickedIndex.isValid())
        {
            auto sourceIndexToEnter = proxyModel()->mapToSource(clickedIndex);
            if(proxyModel()->sourceModel()->canFetchMore(sourceIndexToEnter))
            {
                proxyModel()->setExpandMapped(false);
                proxyModel()->sourceModel()->fetchMore(sourceIndexToEnter);
                return;
            }
        }
        QTreeView::mouseDoubleClickEvent(event);
    }
}

void NodeSelectorTreeView::keyPressEvent(QKeyEvent *event)
{
    if(!selectionModel())
    {
        return;
    }

    QModelIndexList selectedRows = selectionModel()->selectedRows();

    static QModelIndex cdRootIndex = proxyModel()->getIndexFromNode(MegaSyncApp->getRootNode());
    static QList<int> bannedFromRootKeyList = QList<int>() << Qt::Key_Left << Qt::Key_Right
                                                     << Qt::Key_Plus << Qt::Key_Minus;

    if(!bannedFromRootKeyList.contains(event->key()) || !selectedRows.contains(cdRootIndex))
    {
        if(event->key() == Qt::Key_F2)
        {
            renameNode();
        }
        else if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
        {
            if(!selectedRows.isEmpty())
            {
                if(selectedRows.first() == rootIndex() || selectedRows.size() > 1)
                {
                    emit nodeSelected();
                }
                else
                {
                    auto node = std::unique_ptr<MegaNode>(mMegaApi->getNodeByHandle(getSelectedNodeHandle()));
                    if(node)
                    {
                        if(node->isFolder())
                        {
                            emit doubleClicked(selectedRows.first());
                        }
                        else
                        {
                            emit nodeSelected();
                        }
                    }
                }
            }
        }
        else if(event->key() == Qt::Key_Delete)
        {
            auto selectionHandles(getMultiSelectionNodeHandle());

            if(areAllEligibleForDeletion(selectionHandles))
            {
                removeNode(selectionHandles, false);
            }
        }

        QTreeView::keyPressEvent(event);
    }
}

void NodeSelectorTreeView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu customMenu;
    Platform::getInstance()->initMenu(&customMenu, "CustomMenu");

    if (!selectionModel() || !indexAt(event->pos()).isValid())
    {
        return;
    }

    if (selectionModel()->selectedRows().size() == 1)
    {
        QList<mega::MegaHandle> selectionHandle{getSelectedNodeHandle()};
        if (areAllEligibleForRestore(selectionHandle))
        {
            customMenu.addAction(tr("Restore"),
                                 this,
                                 [this, selectionHandle]()
                                 {
                                     restore(selectionHandle);
                                 });
        }

        auto nodeHandle(getSelectedNodeHandle());
        std::unique_ptr<mega::MegaNode> node(mMegaApi->getNodeByHandle(nodeHandle));
        if (node && mMegaApi->isInRubbish(node.get()))
        {
            customMenu.addAction(tr("Delete permanently"),
                                 this,
                                 [this, selectionHandle]()
                                 {
                                     removeNode(selectionHandle, true);
                                 });
        }
        else
        {
            int access = Utilities::getNodeAccess(getSelectedNodeHandle());

            auto proxyModel = static_cast<NodeSelectorProxyModel*>(model());
            if (proxyModel->isNotAProtectedModel() && access != MegaShare::ACCESS_UNKNOWN)
            {
                if (access == MegaShare::ACCESS_OWNER)
                {
                    customMenu.addAction(tr("Get MEGA link"),
                                         this,
                                         &NodeSelectorTreeView::getMegaLink);
                }

                if (access >= MegaShare::ACCESS_FULL)
                {
                    auto sourceModel = proxyModel->getMegaModel();
                    auto index =
                        proxyModel->mapToSource(selectionModel()->selectedIndexes().first());
                    auto item = sourceModel->getItemByIndex(index);

                    if (item)
                    {
                        auto itemStatus = item->getStatus();
                        if (itemStatus == NodeSelectorModelItem::Status::NONE &&
                            !(item->getNode()->isFile()))
                        {
                            customMenu.addAction(tr("Sync"),
                                                 this,
                                                 [selectionHandle]()
                                                 {
                                                     CreateRemoveSyncsManager::addSync(
                                                         selectionHandle.first(),
                                                         true);
                                                 });
                        }
                        else if (itemStatus == NodeSelectorModelItem::Status::SYNC)
                        {
                            customMenu.addAction(tr("Unsync"),
                                                 this,
                                                 [selectionHandle, this]()
                                                 {
                                                     CreateRemoveSyncsManager::removeSync(
                                                         selectionHandle.first(),
                                                         this);
                                                 });
                        }
                    }

                    customMenu.addAction(tr("Rename"), this, &NodeSelectorTreeView::renameNode);
                    customMenu.addAction(tr("Delete"),
                                         this,
                                         [this, selectionHandle]()
                                         {
                                             removeNode(selectionHandle, false);
                                         });
                }
            }
        }
    }
    else if (selectionModel()->selectedRows().size() > 1)
    {
        auto selectionHandles(getMultiSelectionNodeHandle());

        if (!selectionHandles.isEmpty())
        {
            // All or none
            if (areAllEligibleForDeletion(selectionHandles))
            {
                std::unique_ptr<mega::MegaNode> node(
                    mMegaApi->getNodeByHandle(selectionHandles.first()));
                if (node && mMegaApi->isInRubbish(node.get()))
                {
                    customMenu.addAction(tr("Delete permanently"),
                                         this,
                                         [this, selectionHandles]()
                                         {
                                             removeNode(selectionHandles, true);
                                         });
                }
                else
                {
                    customMenu.addAction(tr("Delete"),
                                         this,
                                         [this, selectionHandles]()
                                         {
                                             removeNode(selectionHandles, false);
                                         });
                }
            }

            if (areAllEligibleForRestore(selectionHandles))
            {
                customMenu.addAction(tr("Restore"),
                                     this,
                                     [this, selectionHandles]()
                                     {
                                         restore(selectionHandles);
                                     });
            }
        }
    }

    if (!customMenu.actions().isEmpty())
    {
        customMenu.exec(mapToGlobal(event->pos()));
    }
}

void NodeSelectorTreeView::dragEnterEvent(QDragEnterEvent* event)
{
    if(proxyModel()->getMegaModel()->acceptDragAndDrop(event->mimeData()))
    {
        event->acceptProposedAction();
        event->accept();
    }
}

void NodeSelectorTreeView::dragMoveEvent(QDragMoveEvent* event)
{
    if(proxyModel()->getMegaModel()->acceptDragAndDrop(event->mimeData()))
    {
        // get drop index
        QModelIndex dropIndex = indexAt(event->pos());

        // clear selection and select only the drop index
        selectionModel()->clearSelection();

        if (!proxyModel()->canDropMimeData(event->mimeData(), Qt::CopyAction, -1, -1, dropIndex))
        {
            event->ignore();
            return;
        }

        selectionModel()->select(indexAt(event->pos()), QItemSelectionModel::Select | QItemSelectionModel::Rows);

        event->acceptProposedAction();
        event->accept();
    }
}

void NodeSelectorTreeView::dropEvent(QDropEvent* event)
{
    if(proxyModel()->getMegaModel()->acceptDragAndDrop(event->mimeData()))
    {
        // get drop index
        QModelIndex dropIndex = indexAt(event->pos());

        // Get the list of URLs
        QList<QUrl> urlList = event->mimeData()->urls();
        if(!urlList.isEmpty())
        {
            // get the node handle of the drop index from the proxy model
            auto node = getDropNode(dropIndex);
            if(node)
            {
                MegaSyncApp->uploadFilesToNode(urlList, node->getHandle());
            }
            else
            {
                event->ignore();
                return;
            }
        }

        QTreeView::dropEvent(event);
        event->acceptProposedAction();
    }
}

std::shared_ptr<MegaNode> NodeSelectorTreeView::getDropNode(const QModelIndex& dropIndex)
{
    if(!dropIndex.isValid())
    {
        const auto root = rootIndex();
        NodeSelectorModelItem* item(nullptr);
        if(!root.isValid())
        {
            QModelIndex cdRootIndex = proxyModel()->getIndexFromNode(MegaSyncApp->getRootNode());
            item = NodeSelectorModel::getItemByIndex(cdRootIndex);
        }
        else
        {
            item = NodeSelectorModel::getItemByIndex(root);
        }
        return item ? item->getNode() : nullptr;
    }
    auto node = proxyModel()->getNode(dropIndex);
    if(!node || node->isFolder())
    {
        return node;
    }
    return std::shared_ptr<MegaNode>(mMegaApi->getParentNode(node.get()));
}

bool NodeSelectorTreeView::areAllEligibleForMove(const QList<MegaHandle>& handles,
                                                 const QModelIndex& targetIndex) const
{
    auto item = NodeSelectorModel::getItemByIndex(targetIndex);
    if (!item)
    {
        return false;
    }

    auto targetHandle(item->getNode()->getHandle());

    auto movableItems(handles.size());
    foreach(auto&& nodeHandle, handles)
    {
        std::unique_ptr<mega::MegaNode> node(
            MegaSyncApp->getMegaApi()->getNodeByHandle(nodeHandle));
        if (node && node->getParentHandle() != targetHandle &&
            Utilities::getNodeAccess(nodeHandle) >= MegaShare::ACCESS_FULL)
        {
            movableItems--;
        }
    }

    return movableItems == 0;
}

bool NodeSelectorTreeView::areAllEligibleForDeletion(const QList<MegaHandle> &handles) const
{
    auto removableItems(handles.size());
    for (const auto& handle: handles)
    {
        if (Utilities::getNodeAccess(handle) >= MegaShare::ACCESS_FULL)
        {
            removableItems--;
        }
    }

    return removableItems == 0;
}

bool NodeSelectorTreeView::areAllEligibleForRestore(const QList<MegaHandle> &handles) const
{
    auto restorableItems(handles.size());
    for (const auto& handle: handles)
    {
        std::unique_ptr<mega::MegaNode> node(mMegaApi->getNodeByHandle(handle));
        if (node && mMegaApi->isInRubbish(node.get()))
        {
            std::unique_ptr<mega::MegaNode> parentNode(
                mMegaApi->getNodeByHandle(node->getParentHandle()));
            auto previousParentNode =
                std::shared_ptr<MegaNode>(mMegaApi->getNodeByHandle(node->getRestoreHandle()));

            if (previousParentNode && !mMegaApi->isInRubbish(previousParentNode.get()))
            {
                restorableItems--;
            }
        }
    }

    return restorableItems == 0;
}

void NodeSelectorTreeView::removeNode(const QList<MegaHandle> &handles, bool permanently)
{
    emit removeNodeClicked(handles, permanently);
}

void NodeSelectorTreeView::renameNode()
{
    emit renameNodeClicked();
}

void NodeSelectorTreeView::getMegaLink()
{
    emit getMegaLinkClicked();
}

void NodeSelectorTreeView::restore(const QList<mega::MegaHandle>& handles)
{
    emit restoreClicked(handles);
}

void NodeSelectorTreeView::onNavigateReady(const QModelIndex &index)
{
    if(index.isValid())
    {
        //Loading finished
        proxyModel()->getMegaModel()->sendBlockUiSignal(false);

        QPoint point = visualRect(index).center();
        QMouseEvent mouseEvent(QEvent::MouseButtonDblClick, point, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        mouseDoubleClickEvent(&mouseEvent);
    }
}

bool NodeSelectorTreeView::mousePressorReleaseEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();
    QModelIndex index = indexAt(pos);
    if(!index.isValid())
    {
        return false;
    }

    auto item = qvariant_cast<NodeSelectorModelItem*>(index.data(toInt(NodeSelectorModelRoles::MODEL_ITEM_ROLE)));
    if(item && item->isCloudDrive())
    {   //this line avoid to cloud drive being collapsed and at same time it allows to select it.
       return handleStandardMouseEvent(event);
    }
    else
    {
        if(!index.data(toInt(NodeRowDelegateRoles::INIT_ROLE)).toBool())
        {
            int position = columnViewportPosition(0);
            QModelIndex idx = index.parent();
            while(rootIndex() != idx)
            {
                position += indentation();
                idx = idx.parent();
            }
            QRect rect(position, event->pos().y(), indentation(), rowHeight(index));

            if(rect.contains(event->pos()))
            {
                if(!isExpanded(index))
                {
                    auto sourceIndexToExpand = proxyModel()->mapToSource(index);
                    if(proxyModel()->sourceModel()->canFetchMore(sourceIndexToExpand))
                    {
                        proxyModel()->setExpandMapped(true);
                        proxyModel()->sourceModel()->fetchMore(sourceIndexToExpand);
                    }

                    return handleStandardMouseEvent(event);
                }
            }
        }
    }
    return true;
}

bool NodeSelectorTreeView::handleStandardMouseEvent(QMouseEvent* event)
{
    if(event->type() == QMouseEvent::MouseButtonPress)
    {
        QAbstractItemView::mousePressEvent(event);
    }
    else if(event->type() == QMouseEvent::MouseButtonRelease)
    {
        QAbstractItemView::mouseReleaseEvent(event);
    }

    return false;
}

NodeSelectorTreeViewHeaderView::NodeSelectorTreeViewHeaderView(Qt::Orientation orientation, QWidget *parent) :
    QHeaderView(orientation, parent)
{
    setDefaultAlignment(Qt::AlignLeft);
    setStretchLastSection(true);
    setDefaultSectionSize(35);
}

void NodeSelectorTreeViewHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();
    if(logicalIndex == NodeSelectorModel::USER || logicalIndex == NodeSelectorModel::STATUS)
    {  
        QRect iconRect(QPoint(rect.topLeft()), QSize(18, 18));
        iconRect.moveCenter(rect.center());
        QIcon icon = model()->headerData(logicalIndex, Qt::Orientation::Horizontal, toInt(HeaderRoles::ICON_ROLE)).value<QIcon>();
        if(!icon.isNull())
        {
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
            icon.paint(painter, iconRect, Qt::AlignVCenter | Qt::AlignHCenter);
        }
    }
}

