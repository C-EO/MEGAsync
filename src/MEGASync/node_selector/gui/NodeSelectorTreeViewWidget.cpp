#include "NodeSelectorTreeViewWidget.h"

#include "EventUpdater.h"
#include "MegaApplication.h"
#include "MegaNodeNames.h"
#include "NodeSelectorDelegates.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorProxyModel.h"
#include "NodeSelectorSelectionCoordinator.h"
#include "NodeSelectorTreeViewWidgetSpecializations.h"
#include "RequestListenerManager.h"
#include "TokenizableItems/TokenPropertySetter.h"
#include "ui_NodeSelectorTreeViewWidget.h"

#include <QKeyEvent>

#include <algorithm>

const int CHECK_UPDATED_NODES_INTERVAL = 1000;
const int IMMEDIATE_CHECK_UPDATES_NODES_THRESHOLD = 200;

NodeSelectorTreeViewWidget::NodeSelectorTreeViewWidget(SelectTypeSPtr mode, QWidget* parent):
    QWidget(parent),
    ui(new Ui::NodeSelectorTreeViewWidget),
    mProxyModel(nullptr),
    mModel(nullptr),
    mNavigation(MegaSyncApp->getMegaApi()),
    mNodeActions(MegaSyncApp->getMegaApi()),
    mMegaApi(MegaSyncApp->getMegaApi()),
    mSelectType(mode),
    mManuallyResizedColumn(false),
    mResizeEventsReceived(0),
    first(true),
    mUiBlocked(false),
    mWasEmpty(true)
{
    ui->setupUi(this);
    setFocusProxy(ui->tMegaFolders);
    ui->searchButtonsWidget->setVisible(false);

    connect(&ui->tMegaFolders->loadingView(),
            &ViewLoadingSceneBase::sceneVisibilityChange,
            this,
            &NodeSelectorTreeViewWidget::onUiBlocked);

    // Chip selector configuration
    BaseTokens iconTokens;
    iconTokens.setNormalOff(QLatin1String("icon-primary"));
    iconTokens.setNormalOn(QLatin1String("brand-on-container"));
    auto iconTokensSetter = std::make_shared<TokenPropertySetter>(iconTokens);
    TabSelector::applyTokens(ui->searchButtonsWidget, iconTokensSetter);

    ui->cloudDriveSearch->setProperty("title", MegaNodeNames::getCloudDriveName());
    ui->backupsSearch->setProperty("title", MegaNodeNames::getBackupsName());
    ui->incomingSharesSearch->setProperty("title", MegaNodeNames::getIncomingSharesName());
    ui->rubbishSearch->setProperty("title", MegaNodeNames::getRubbishName());

    mResizeEventsTimer.setSingleShot(true);
    mResizeEventsTimer.setInterval(10);
    connect(&mResizeEventsTimer,
            &QTimer::timeout,
            this,
            [this]()
            {
                if (mResizeEventsReceived > 3)
                {
                    mManuallyResizedColumn = true;
                }

                mResizeEventsReceived = 0;
            });

    // Empty pages
    ui->emptyPage->installEventFilter(this);
    ui->emptyFolderPage->installEventFilter(this);
    ui->emptyPage->setFocusPolicy(Qt::StrongFocus);
    ui->emptyFolderPage->setFocusPolicy(Qt::StrongFocus);
    // By default, the empty pages don´t allow drag and drop.
    // emptyFolderPage can accept drops if "enableDragAndDrop" is called with true
    ui->emptyFolderPage->setAcceptDrops(false);
    ui->emptyPage->setAcceptDrops(false);

    mNodeActions.setDialogParent(Utilities::getTopParent<QDialog>(ui->tMegaFolders));
}

NodeSelectorTreeViewWidget::~NodeSelectorTreeViewWidget()
{
    delete ui;
}

void NodeSelectorTreeViewWidget::init()
{
    // When init, show the loading view and then, add a 150 delay to avoid showing the loading view
    // for short loads
    setLoadingSceneVisible(true);
    ui->tMegaFolders->loadingView().setDelayTimeToShowInMs(150);
    mProxyModel = createProxyModel();
    mModel = createModel();
    mSelectionCoordinator = std::make_unique<NodeSelectorSelectionCoordinator>(
        mMegaApi,
        NodeSelectorSelectionCoordinator::Objects{
            mModel.get(),
            mProxyModel.get(),
            ui->tMegaFolders,
        },
        NodeSelectorSelectionCoordinator::Policy{
            [this](const QModelIndex& index, bool setCurrent, bool exclusiveSelect)
            {
                selectIndex(index, setCurrent, exclusiveSelect);
            },
            [this]()
            {
                clearSelection();
            },
            [this](const QModelIndex& index)
            {
                onItemDoubleClick(index);
            },
            [this]()
            {
                setRootIndex(mProxyModel->getTopRootIndex());
            },
            [this](std::function<bool()> selectionOperation)
            {
                disconnect(ui->tMegaFolders->selectionModel(),
                           &QItemSelectionModel::selectionChanged,
                           this,
                           &NodeSelectorTreeViewWidget::onSelectionChanged);
                const bool shouldNotify = selectionOperation();
                connect(ui->tMegaFolders->selectionModel(),
                        &QItemSelectionModel::selectionChanged,
                        this,
                        &NodeSelectorTreeViewWidget::onSelectionChanged);
                if (shouldNotify)
                {
                    onSelectionChanged(QItemSelection(), QItemSelection());
                }
            },
        });

    auto updateObjects = NodeSelectorModelUpdateCoordinator::Objects{
        mModel.get(),
        mProxyModel.get(),
        ui->tMegaFolders,
    };

    auto updateSharedState = NodeSelectorModelUpdateCoordinator::SharedState{
        mSelectionCoordinator->movedHandlesToSelect(),
        mSelectionCoordinator->parentOfRestoredNodes(),
        mSelectionCoordinator->mergeTargetFolders(),
        mNodesToBeReplaced,
    };

    auto updatePolicy = NodeSelectorModelUpdateCoordinator::Policy{
        [this](const QModelIndex& index, mega::MegaNode* node)
        {
            return static_cast<int>(getNodeOnModelState(index, node));
        },
        [this](mega::MegaHandle parentHandle)
        {
            return getAddedNodeParent(parentHandle);
        },
    };

    mModelUpdateCoordinator =
        std::make_unique<NodeSelectorModelUpdateCoordinator>(mMegaApi,
                                                             updateObjects,
                                                             updateSharedState,
                                                             std::move(updatePolicy));

    connect(mModelUpdateCoordinator.get(),
            &NodeSelectorModelUpdateCoordinator::indexRemovedFromHistory,
            this,
            &NodeSelectorTreeViewWidget::onRemoveIndexFromGoBack);
    connect(mModelUpdateCoordinator.get(),
            &NodeSelectorModelUpdateCoordinator::handleRemovedFromNavigation,
            this,
            [this](mega::MegaHandle handle)
            {
                mNavigation.onHandleRemoved(handle);
            });
    connect(mModelUpdateCoordinator.get(),
            &NodeSelectorModelUpdateCoordinator::viewStateChanged,
            this,
            &NodeSelectorTreeViewWidget::notifyViewStateChanged);

    mNodeActions.setModel(mModel.get());

    // Regardless the type of treeviewwidget, the empty icon always use icon-secondary token
    ui->emptyIcon->setProperty(TOKEN_PROPERTIES::normalOff, QLatin1String("icon-secondary"));
    ui->emptyIcon->setIcon(getEmptyIcon());

    initEmptyMessages();

    mSelectType->init(this);

    ui->tMegaFolders->setSortingEnabled(true);
    ui->tMegaFolders->setAllowContextMenu(mSelectType->isContextMenuAllowed());
    ui->tMegaFolders->setAllowNewFolderContextMenuItem(mSelectType->hasNewFolderButton());
    ui->tMegaFolders->viewport()->installEventFilter(this);

    mProxyModel->setSourceModel(mModel.get());

    connect(mProxyModel.get(),
            &NodeSelectorProxyModel::levelLoaded,
            this,
            &NodeSelectorTreeViewWidget::onLevelLoaded);
    connect(mProxyModel.get(),
            &NodeSelectorProxyModel::modelSorted,
            this,
            &NodeSelectorTreeViewWidget::checkViewOnModelChange);
    connect(mModel.get(),
            &QAbstractItemModel::rowsInserted,
            this,
            &NodeSelectorTreeViewWidget::checkViewOnModelChange);
    connect(mModel.get(),
            &QAbstractItemModel::rowsRemoved,
            this,
            &NodeSelectorTreeViewWidget::checkViewOnModelChange);
    connect(mModel.get(),
            &NodeSelectorModel::blockUi,
            this,
            &NodeSelectorTreeViewWidget::setLoadingSceneVisible);
    connect(mModel.get(),
            &NodeSelectorModel::modelModified,
            this,
            &NodeSelectorTreeViewWidget::onModelModified);
    connect(mModel.get(),
            &NodeSelectorModel::itemsMoved,
            mSelectionCoordinator.get(),
            &NodeSelectorSelectionCoordinator::onItemsMoved);
    connect(mModel.get(),
            &NodeSelectorModel::nodesAdded,
            mSelectionCoordinator.get(),
            &NodeSelectorSelectionCoordinator::onNodesAdded);

#ifdef __APPLE__
    ui->tMegaFolders->setAnimated(false);
#endif

    connect(&mNodesUpdateTimer,
            &QTimer::timeout,
            this,
            &NodeSelectorTreeViewWidget::processCachedNodesUpdated);
    mNodesUpdateTimer.start(CHECK_UPDATED_NODES_INTERVAL);
}

void NodeSelectorTreeViewWidget::initEmptyMessages()
{
    auto emptyLabelInfo(getEmptyLabel());
    ui->descriptionEmptyLabel->hide();
    ui->titleEmptyLabel->hide();
    if (!emptyLabelInfo.description.isEmpty())
    {
        ui->descriptionEmptyLabel->show();
        ui->descriptionEmptyLabel->setText(emptyLabelInfo.description);
    }
    if (!emptyLabelInfo.title.isEmpty())
    {
        ui->titleEmptyLabel->show();
        ui->titleEmptyLabel->setText(emptyLabelInfo.title);
    }
}

bool NodeSelectorTreeViewWidget::event(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        initEmptyMessages();

        if (mModel && mProxyModel && mSelectType)
        {
            setEmptyFolderPage();
        }
    }
    else if (event->type() == QEvent::MouseButtonRelease)
    {
        clearSelection();
    }

    return QWidget::event(event);
}

bool NodeSelectorTreeViewWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::DragEnter)
    {
        if (auto dropEvent = static_cast<QDragEnterEvent*>(event))
        {
            auto proxyIndex = ui->tMegaFolders->indexAt(dropEvent->pos());
            auto sourceIndex = mProxyModel->mapToSource(proxyIndex);

            if (!dropEvent->mimeData()->urls().isEmpty())
            {
                ui->tMegaFolders->dragEnterEvent(dropEvent);
            }
            else if (mModel->canDropMimeData(dropEvent->mimeData(),
                                             Qt::MoveAction,
                                             sourceIndex.row(),
                                             sourceIndex.column(),
                                             sourceIndex.parent()))
            {
                dropEvent->acceptProposedAction();
            }
        }
    }
    else if (event->type() == QEvent::DragMove)
    {
        if (auto moveEvent = static_cast<QDragMoveEvent*>(event))
        {
            if (!moveEvent->mimeData()->urls().isEmpty())
            {
                ui->tMegaFolders->dragMoveEvent(moveEvent);
            }
        }
    }
    else if (event->type() == QEvent::Drop &&
             ui->stackedWidget->currentWidget() != ui->treeViewPage)
    {
        dropIntoRootIndex(static_cast<QDropEvent*>(event));
    }
    else if (event->type() == QEvent::Resize)
    {
        if (watched == ui->tMegaFolders->viewport())
        {
            updateColumnsWidth(false);
        }
    }
    // Propagate key events to the view
    else if ((watched == ui->emptyPage || watched == ui->emptyFolderPage) &&
             (event->type() == QEvent::ShortcutOverride || event->type() == QEvent::KeyPress))
    {
        if (auto keyEvent = static_cast<QKeyEvent*>(event); keyEvent->matches(QKeySequence::Paste))
        {
            if (event->type() == QEvent::ShortcutOverride)
            {
                event->accept();
            }
            else
            {
                QMetaObject::invokeMethod(ui->tMegaFolders,
                                          "onPasteShortcutActivated",
                                          Qt::DirectConnection);
            }

            return true;
        }
    }
    else if (watched == ui->emptyFolderPage && event->type() == QEvent::ContextMenu)
    {
        ui->tMegaFolders->contextMenuEvent(static_cast<QContextMenuEvent*>(event));
    }

    return QWidget::eventFilter(watched, event);
}

void NodeSelectorTreeViewWidget::clearSelection()
{
    ui->tMegaFolders->clearSelection();
}

bool NodeSelectorTreeViewWidget::isSelectionCorrect()
{
    if (ui->tMegaFolders->selectionModel())
    {
        return mSelectType->okButtonEnabled(this, ui->tMegaFolders->selectedRows());
    }
    return false;
}

void NodeSelectorTreeViewWidget::abort()
{
    mModel->abort();
}

NodeSelectorModelItem* NodeSelectorTreeViewWidget::rootItem()
{
    auto rootIndex = ui->tMegaFolders->rootIndex();
    if (!rootIndex.isValid())
    {
        // Top parent
        rootIndex = mModel->index(0, 0, QModelIndex());
    }

    return mModel->getItemByIndex(rootIndex);
}

QModelIndex NodeSelectorTreeViewWidget::getCurrentRootIndex()
{
    return ui->tMegaFolders->rootIndex();
}

NodeSelectorProxyModel* NodeSelectorTreeViewWidget::getProxyModel()
{
    return mProxyModel.get();
}

bool NodeSelectorTreeViewWidget::canGoBack() const
{
    return mNavigation.canGoBack();
}

bool NodeSelectorTreeViewWidget::canGoForward() const
{
    return mNavigation.canGoForward();
}

bool NodeSelectorTreeViewWidget::shouldShowNavigationButtons() const
{
    return mNavigation.shouldShowNavigationButtons();
}

bool NodeSelectorTreeViewWidget::isInRootView() const
{
    return !ui->tMegaFolders->rootIndex().isValid();
}

bool NodeSelectorTreeViewWidget::isEmpty() const
{
    return ui->tMegaFolders->model() ?
               ui->tMegaFolders->model()->rowCount(mProxyModel->getTopRootIndex()) == 0 :
               true;
}

void NodeSelectorTreeViewWidget::enableDragAndDrop(bool enable)
{
    ui->emptyFolderPage->setAcceptDrops(enable);
    ui->emptyPage->setAcceptDrops(enable);
    ui->tMegaFolders->setDragEnabled(enable);
    ui->tMegaFolders->viewport()->setAcceptDrops(enable);
    ui->tMegaFolders->setDropIndicatorShown(enable);
    ui->tMegaFolders->setDragDropMode(enable ? QAbstractItemView::DragDrop :
                                               QAbstractItemView::NoDragDrop);
}

void NodeSelectorTreeViewWidget::goBack()
{
    if (mUiBlocked)
    {
        return;
    }

    const auto targetHandle = mNavigation.goBack(getHandleByIndex(ui->tMegaFolders->rootIndex()));
    if (!targetHandle.has_value())
    {
        return;
    }

    const auto rootIndex = ui->tMegaFolders->rootIndex();
    setRootIndex(getIndexFromHandle(*targetHandle));

    if (rootIndex.isValid())
    {
        selectIndex(rootIndex, true, true);
    }
}

void NodeSelectorTreeViewWidget::goForward()
{
    if (mUiBlocked)
    {
        return;
    }

    const auto targetHandle =
        mNavigation.goForward(getHandleByIndex(ui->tMegaFolders->rootIndex()));
    if (!targetHandle.has_value())
    {
        return;
    }

    setRootIndex(getIndexFromHandle(*targetHandle));
    onSelectionHasChanged();
}

void NodeSelectorTreeViewWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::BackButton && mNavigation.canGoBack() && !mUiBlocked)
    {
        goBack();
    }
    else if (event->button() == Qt::ForwardButton && mNavigation.canGoForward() && !mUiBlocked)
    {
        goForward();
    }
}

void NodeSelectorTreeViewWidget::onRootIndexChanged(const QModelIndex&)
{
    updateColumnsWidth(true);
}

void NodeSelectorTreeViewWidget::updateColumnsWidth(bool updateVisibleColumnCounter)
{
    if (updateVisibleColumnCounter)
    {
        mVisibleColumns.clear();

        for (int column = 0; column < ui->tMegaFolders->header()->count(); ++column)
        {
            if (!ui->tMegaFolders->header()->isSectionHidden(column))
            {
                mVisibleColumns.append(column);
            }
        }
    }

    if (!mVisibleColumns.isEmpty() && !mManuallyResizedColumn)
    {
        int widthTotal(0);
        int minWidth(100);
        int maxSecondaryColumnWidth(200);
        double secondaryColumnProportion(0.2);

        for (QList<int>::const_reverse_iterator column = mVisibleColumns.crbegin();
             column != mVisibleColumns.crend();
             ++column)
        {
            int width(0);

            if ((*column) == NodeSelectorModel::Column::NODE)
            {
                // Total minus the rest of columns
                width = std::max(ui->tMegaFolders->viewport()->width() - widthTotal, minWidth * 2);
            }
            else
            {
                width =
                    std::max(std::min(qRound(ui->tMegaFolders->width() * secondaryColumnProportion),
                                      maxSecondaryColumnWidth),
                             minWidth);
                widthTotal += width;
            }

            ui->tMegaFolders->setColumnWidth((*column), width);
        }
    }
}

void NodeSelectorTreeViewWidget::onSectionResized()
{
    if (!mManuallyResizedColumn && ui->tMegaFolders->header()->rect().contains(
                                       ui->tMegaFolders->mapFromGlobal(QCursor::pos())))
    {
        mResizeEventsReceived++;

        // Protect against clicking on the header to show the sort indicator.
        // Only if the event is received 3 times in an span of 10ms, it is a real resize
        if (!mResizeEventsTimer.isActive())
        {
            mResizeEventsTimer.start();
        }
    }
}

void NodeSelectorTreeViewWidget::checkViewOnModelChange()
{
    setEmptyFolderPage();
}

void NodeSelectorTreeViewWidget::setNewFolderInfo(const NewFolderInfo& newNewFolderInfo)
{
    mSelectionCoordinator->setNewFolderInfo(newNewFolderInfo);
}

void NodeSelectorTreeViewWidget::onLevelLoaded()
{
    // Initialise the view only the first time, but always refresh the empty/nav state below
    if (ui->tMegaFolders->model() == nullptr)
    {
        ui->tMegaFolders->setContextMenuPolicy(Qt::DefaultContextMenu);
        ui->tMegaFolders->setExpandsOnDoubleClick(false);
        ui->tMegaFolders->header()->setDefaultAlignment(Qt::AlignLeft);
        ui->tMegaFolders->header()->setDefaultSectionSize(35);
        ui->tMegaFolders->setItemDelegate(new NodeRowDelegate(ui->tMegaFolders));
        ui->tMegaFolders->setTextElideMode(Qt::ElideMiddle);

        ui->tMegaFolders->sortByColumn(NodeSelectorModel::Column::NODE, Qt::AscendingOrder);
        ui->tMegaFolders->setModel(mProxyModel.get());

        ui->tMegaFolders->header()->setVisible(true);
        ui->tMegaFolders->header()->setProperty("HeaderIconCenter", true);

        // those connects needs to be done after the model is set, do not move them
        connect(ui->tMegaFolders->selectionModel(),
                &QItemSelectionModel::selectionChanged,
                this,
                &NodeSelectorTreeViewWidget::onSelectionChanged);
        connect(ui->tMegaFolders,
                &NodeSelectorTreeView::deleteNodeClicked,
                this,
                &NodeSelectorTreeViewWidget::onDeleteClicked);
        connect(ui->tMegaFolders,
                &NodeSelectorTreeView::leaveShareClicked,
                this,
                &NodeSelectorTreeViewWidget::onLeaveShareClicked);
        connect(ui->tMegaFolders,
                &NodeSelectorTreeView::renameNodeClicked,
                this,
                &NodeSelectorTreeViewWidget::onRenameClicked);
        connect(ui->tMegaFolders,
                &NodeSelectorTreeView::newFolderClicked,
                this,
                [this]()
                {
                    emit onCustomButtonClicked(SelectType::ButtonId::NewFolder);
                });
        connect(ui->tMegaFolders,
                &NodeSelectorTreeView::uploadClicked,
                this,
                [this]()
                {
                    emit onCustomButtonClicked(SelectType::ButtonId::Upload);
                });
        connect(ui->tMegaFolders,
                &NodeSelectorTreeView::getMegaLinkClicked,
                this,
                &NodeSelectorTreeViewWidget::onGenMEGALinkClicked);
        connect(ui->tMegaFolders,
                &QTreeView::doubleClicked,
                this,
                &NodeSelectorTreeViewWidget::onItemDoubleClick);
        connect(ui->tMegaFolders,
                &NodeSelectorTreeView::enterKeyPressed,
                this,
                &NodeSelectorTreeViewWidget::enterKeyPressed);
        connect(ui->tMegaFolders->header(),
                &QHeaderView::sectionResized,
                this,
                &NodeSelectorTreeViewWidget::onSectionResized);

        makeCustomConnections();

        setRootIndex(mModel->hasTopRootIndex() ? mProxyModel->index(0, 0) : QModelIndex());

        setStyleSheet(styleSheet());

        // View ready to work with it > View init and model loaded
        emit viewReady();
    }

    // Check empty view page and forward/backward navigation buttons
    checkViewOnModelChange();
}

void NodeSelectorTreeViewWidget::onRemoveIndexFromGoBack(const QModelIndex& indexToRemove)
{
    if (indexToRemove.isValid())
    {
        auto changeRootIndex = [this](QModelIndex removedIndex)
        {
            auto parentIndex(removedIndex.parent());

            // Avoid adding the cloud drive
            if (parentIndex.parent().isValid())
            {
                setRootIndex(parentIndex);
            }
            else
            {
                setRootIndex(mModel->hasTopRootIndex() ? mProxyModel->index(0, 0) : QModelIndex());
                mNavigation.clearBackward();
            }
        };

        if (indexToRemove == ui->tMegaFolders->rootIndex())
        {
            changeRootIndex(indexToRemove);
        }
        else
        {
            // If the index is in the list of backward handles
            // set the parent as root index and remove the parent from the list of backward handles
            auto indexHandleToRemove(getHandleByIndex(indexToRemove));
            if (mNavigation.hasBackwardHandle(indexHandleToRemove))
            {
                changeRootIndex(indexToRemove);
            }
        }
    }
}

MegaHandle NodeSelectorTreeViewWidget::getHandleByIndex(const QModelIndex& idx)
{
    return mProxyModel ? mProxyModel->getHandle(idx) : mega::INVALID_HANDLE;
}

void NodeSelectorTreeViewWidget::addHandleToBeReplaced(mega::MegaHandle handle)
{
    mNodesToBeReplaced.insert(handle);
}

QModelIndex NodeSelectorTreeViewWidget::getIndexFromHandle(const mega::MegaHandle& handle)
{
    return mProxyModel ? mProxyModel->getIndexFromHandle(handle) : QModelIndex();
}

QModelIndex NodeSelectorTreeViewWidget::getRootIndexFromIndex(const QModelIndex& index)
{
    QModelIndex parentIndex(index);
    while (parentIndex.parent().isValid())
    {
        parentIndex = parentIndex.parent();
    }
    return parentIndex;
}

bool NodeSelectorTreeViewWidget::isAllowedToEnterInIndex(const QModelIndex& idx)
{
    return mSelectType->isAllowedToNavigateInside(idx);
}

void NodeSelectorTreeViewWidget::onItemDoubleClick(const QModelIndex& index)
{
    if (!isAllowedToEnterInIndex(index))
    {
        auto item = mModel->getItemByIndex(index);
        if (item && item->getNode()->isFile() && !item->isTakenDown())
        {
            MegaSyncApp->downloadACtionClickedWithHandles(QList<mega::MegaHandle>()
                                                          << item->getNode()->getHandle());
        }
        return;
    }

    mNavigation.onNavigateInto(getHandleByIndex(ui->tMegaFolders->rootIndex()),
                               mProxyModel->getHandle(index));

    setRootIndex(index);
}

std::shared_ptr<NodeSelectorProxyModel> NodeSelectorTreeViewWidget::createProxyModel()
{
    return mSelectType->createProxyModel();
}

void NodeSelectorTreeViewWidget::setLoadingSceneVisible(bool blockUi)
{
    if (blockUi && ui->stackedWidget->currentWidget() != ui->treeViewPage)
    {
        ui->stackedWidget->setCurrentWidget(ui->treeViewPage);
    }

    ui->tMegaFolders->loadingView().toggleLoadingScene(blockUi);

    if (!blockUi)
    {
        setEmptyFolderPage();
        mSelectionCoordinator->expandPendingIndexes();
        mSelectionCoordinator->selectPendingIndexes();
    }
}

void NodeSelectorTreeViewWidget::selectPendingIndexes()
{
    mSelectionCoordinator->selectPendingIndexes();
}

void NodeSelectorTreeViewWidget::setViewPage()
{
    if (mModel)
    {
        auto topRootIndex = mModel->hasTopRootIndex() ? mModel->index(0, 0) : QModelIndex();

        if (mModel->rowCount(topRootIndex) == 0 && showEmptyView())
        {
            ui->stackedWidget->setCurrentWidget(ui->emptyPage);
            return;
        }
    }
    ui->stackedWidget->setCurrentWidget(ui->treeViewPage);
}

QModelIndex NodeSelectorTreeViewWidget::getAddedNodeParent(mega::MegaHandle parentHandle)
{
    return mModel->findIndexByNodeHandle(parentHandle, QModelIndex());
}

void NodeSelectorTreeViewWidget::onUiBlocked(bool state)
{
    if (mUiBlocked != state)
    {
        mUiBlocked = state;
    }

    emit uiIsBlocked(mUiBlocked);
    ui->searchButtonsWidget->setDisabled(state);

    if (!state)
    {
        onSelectionHasChanged();
    }
}

void NodeSelectorTreeViewWidget::onSelectionChanged(const QItemSelection& selected,
                                                    const QItemSelection& deselected)
{
    Q_UNUSED(deselected)
    Q_UNUSED(selected)

    if (!mUiBlocked)
    {
        onSelectionHasChanged();
    }
}

void NodeSelectorTreeViewWidget::onModelModified()
{
    const bool rootDeleted = !ui->tMegaFolders->rootIndex().isValid() && mNavigation.canGoBack();

    if (rootDeleted)
    {
        emit viewStateChanged();
        return;
    }

    const bool nowEmpty = (mProxyModel->rowCount(getCurrentRootIndex()) == 0);
    if (nowEmpty != mWasEmpty)
    {
        mWasEmpty = nowEmpty;
        setEmptyFolderPage();
        emit viewButtonsStateChanged();
    }
}

void NodeSelectorTreeViewWidget::onSelectionHasChanged()
{
    emit selectionHasChanged();
}

void NodeSelectorTreeViewWidget::onRenameClicked()
{
    mNodeActions.renameNode(getSelectedNodeHandle());
}

void NodeSelectorTreeViewWidget::onDeleteClicked(const QList<mega::MegaHandle>& handles,
                                                 bool permanently,
                                                 bool showConfirmationMessageBox)
{
    mNodeActions.deleteNodes(handles, permanently, showConfirmationMessageBox);
}

void NodeSelectorTreeViewWidget::onLeaveShareClicked(const QList<mega::MegaHandle>& handles)
{
    mNodeActions.leaveShare(handles);
}

NodeSelectorTreeViewWidget::NodeState
    NodeSelectorTreeViewWidget::getNodeOnModelState(const QModelIndex& index, mega::MegaNode* node)
{
    NodeState result(NodeState::DOESNT_EXIST);

    auto parentIndex = mModel->findIndexByNodeHandle(node->getParentHandle(), QModelIndex());

    if (parentIndex.isValid())
    {
        auto parentItem = mModel->getItemByIndex(parentIndex);
        if (parentItem->areChildrenInitialized())
        {
            if (index.parent() == parentIndex)
            {
                result = NodeState::EXISTS;
            }
            else if (index.isValid())
            {
                result = NodeState::MOVED;
            }
            else
            {
                result = NodeState::ADD;
            }
        }
        else
        {
            if (index.isValid())
            {
                result = NodeState::MOVED_OUT_OF_VIEW;
            }
            else
            {
                result = NodeState::EXISTS_BUT_PARENT_UNINITIALISED;
            }
        }
    }
    else if (index.isValid())
    {
        result = NodeState::REMOVE;
    }
    else if (isNodeCompatibleWithModel(node))
    {
        result = NodeState::EXISTS_BUT_OUT_OF_VIEW;
    }

    return result;
}

bool NodeSelectorTreeViewWidget::onNodesUpdate(mega::MegaApi*, mega::MegaNodeList* nodes)
{
    if (!mModelUpdateCoordinator)
    {
        return false;
    }

    if (!mModelUpdateCoordinator->onNodesUpdate(nodes))
    {
        return false;
    }

    if (mModelUpdateCoordinator->shouldUpdateImmediately(IMMEDIATE_CHECK_UPDATES_NODES_THRESHOLD))
    {
        if (mNodesUpdateTimer.interval() != 0)
        {
            mNodesUpdateTimer.setInterval(0);
        }
    }
    else if (mNodesUpdateTimer.interval() != CHECK_UPDATED_NODES_INTERVAL)
    {
        mNodesUpdateTimer.setInterval(CHECK_UPDATED_NODES_INTERVAL);
    }

    return true;
}

void NodeSelectorTreeViewWidget::selectIndex(const mega::MegaHandle& handle,
                                             bool setCurrent,
                                             bool exclusiveSelect)
{
    auto index(mProxyModel->getIndexFromHandle(handle));
    if (index.isValid())
    {
        selectIndex(index, setCurrent, exclusiveSelect);
    }
}

void NodeSelectorTreeViewWidget::selectIndex(const QModelIndex& index,
                                             bool setCurrent,
                                             bool exclusiveSelect)
{
    auto selectionFlag(exclusiveSelect ? QItemSelectionModel::ClearAndSelect :
                                         QItemSelectionModel::Select);

    auto selectionModel = ui->tMegaFolders->selectionModel();
    if (selectionModel != nullptr)
    {
        if (setCurrent)
        {
            selectionModel->setCurrentIndex(index, selectionFlag | QItemSelectionModel::Rows);
        }

        selectionModel->select(index, selectionFlag | QItemSelectionModel::Rows);
    }
    else
    {
        mega::MegaApi::log(
            mega::MegaApi::LOG_LEVEL_ERROR,
            QString::fromUtf8("Invalid selectionModel access.").toUtf8().constData());
    }

    ui->tMegaFolders->scrollTo(index, QAbstractItemView::ScrollHint::PositionAtCenter);
}

bool NodeSelectorTreeViewWidget::increaseMovingNodes(int number)
{
    mSelectionCoordinator->resetMoveNodesToSelect();
    return mModel->increaseMovingNodes(number);
}

bool NodeSelectorTreeViewWidget::decreaseMovingNodes(int number)
{
    return mModel->moveProcessedByNumber(number);
}

void NodeSelectorTreeViewWidget::finishMovingNodes()
{
    mModel->finishMovingNodes();
}

bool NodeSelectorTreeViewWidget::areItemsAboutToBeMovedFromHere(mega::MegaHandle firstHandleMoved)
{
    auto itemIndex(mModel->findIndexByNodeHandle(firstHandleMoved, QModelIndex()));
    if (itemIndex.isValid())
    {
        return true;
    }

    return false;
}

void NodeSelectorTreeViewWidget::processCachedNodesUpdated()
{
    if (mModelUpdateCoordinator)
    {
        mModelUpdateCoordinator->processCachedNodesUpdated();
    }
}

void NodeSelectorTreeViewWidget::setParentOfRestoredNodes(
    const QSet<mega::MegaHandle>& parentOfRestoredNodes)
{
    mSelectionCoordinator->setParentOfRestoredNodes(parentOfRestoredNodes);
}

void NodeSelectorTreeViewWidget::setMergeFolderHandles(
    const QMultiHash<SourceHandle, TargetHandle>& handles)
{
    mSelectionCoordinator->setMergeFolderHandles(handles);
}

void NodeSelectorTreeViewWidget::resetMergeFolderHandles(
    const QMultiHash<SourceHandle, TargetHandle>& handles)
{
    mSelectionCoordinator->resetMergeFolderHandles(handles);
}

bool NodeSelectorTreeViewWidget::isUiBlocked()
{
    return mUiBlocked;
}

void NodeSelectorTreeViewWidget::dropIntoRootIndex(QDropEvent* event)
{
    if (!event->mimeData()->urls().isEmpty() || mModel->canDropMimeData(event->mimeData(),
                                                                        Qt::MoveAction,
                                                                        -1,
                                                                        -1,
                                                                        mModel->getTopRootIndex()))
    {
        ui->tMegaFolders->dropEvent(event);
    }
}

void NodeSelectorTreeViewWidget::setSelectedNodeHandle(const MegaHandle& selectedHandle)
{
    mSelectionCoordinator->setSelectedNodeHandle(selectedHandle);
}

MegaHandle NodeSelectorTreeViewWidget::getSelectedNodeHandle()
{
    return ui->tMegaFolders->getSelectedNodeHandle();
}

QList<MegaHandle> NodeSelectorTreeViewWidget::getMultiSelectionNodeHandle()
{
    auto selectedRows(ui->tMegaFolders->selectedRows());
    return ui->tMegaFolders->getMultiSelectionNodeHandle(selectedRows);
}

QModelIndexList NodeSelectorTreeViewWidget::getSelectedIndexes() const
{
    return ui->tMegaFolders->selectedRows();
}

bool NodeSelectorTreeViewWidget::containsTakenDownSelected() const
{
    return ui->tMegaFolders->containsTakenDownItem(ui->tMegaFolders->selectedRows());
}

void NodeSelectorTreeViewWidget::notifyViewStateChanged()
{
    emit viewStateChanged();
}

void NodeSelectorTreeViewWidget::notifyButtonsStateChanged()
{
    emit viewButtonsStateChanged();
}

void NodeSelectorTreeViewWidget::setRootIndex(const QModelIndex& proxy_idx)
{
    // In case the idx is coming from a potentially hidden column, we always take the NODE column
    // As it is the only one that have childrens
    auto node_column_idx = proxy_idx.sibling(proxy_idx.row(), NodeSelectorModel::Column::NODE);

    mModel->setCurrentRootIndex(mProxyModel->mapToSource(node_column_idx));
    ui->tMegaFolders->setRootIndex(node_column_idx);
    ui->tMegaFolders->setRootIndexReadOnly(isCurrentRootIndexReadOnly());
    if (ui->tMegaFolders->rootIndex().isValid())
    {
        ui->tMegaFolders->selectionModel()->select(ui->tMegaFolders->rootIndex(),
                                                   QItemSelectionModel::ClearAndSelect);
    }

    // Remove in case the rootindex is in the backward list
    mNavigation.onRootChanged(getHandleByIndex(node_column_idx));

    onRootIndexChanged(node_column_idx);
    setEmptyFolderPage();
    notifyViewStateChanged();
}

QIcon NodeSelectorTreeViewWidget::getEmptyIcon()
{
    return QIcon();
}

void NodeSelectorTreeViewWidget::setEmptyFolderPage()
{
    auto currentRootIndex(getCurrentRootIndex());
    auto topRootIndex(mProxyModel->getTopRootIndex());

    // If we are inside a folder, show the "Empty folder" page.
    if (currentRootIndex != topRootIndex && mProxyModel->rowCount(currentRootIndex) == 0)
    {
        ui->stackedWidget->setCurrentWidget(ui->emptyFolderPage);
        auto emptyFolderInfo = mSelectType->getEmptyFolderPageInfo();
        if (emptyFolderInfo.isValid())
        {
            // By default, it is hidden
            ui->titleEmptyFolderLabel->setVisible(true);
            ui->titleEmptyFolderLabel->setText(emptyFolderInfo.title);
            ui->descriptionEmptyFolderLabel->setText(emptyFolderInfo.description);
            ui->emptyFolderIcon->setIcon(emptyFolderInfo.icon);
            ui->emptyFolderIcon->setIsTokenized(emptyFolderInfo.iconTokenized);
        }
    }
    else
    {
        setViewPage();
    }

    if (ui->stackedWidget->currentWidget() != ui->treeViewPage)
    {
        ui->stackedWidget->currentWidget()->setFocus(Qt::OtherFocusReason);
    }
}

NodeSelectorTreeViewWidget::EmptyLabelInfo NodeSelectorTreeViewWidget::getEmptyLabel()
{
    return EmptyLabelInfo();
}

QModelIndex NodeSelectorTreeViewWidget::getParentIncomingShareByIndex(QModelIndex idx)
{
    while (idx.isValid())
    {
        if (auto item = NodeSelectorModel::getItemByIndex(idx))
        {
            if (item->getNode()->isInShare())
            {
                return idx;
            }
            else
            {
                idx = idx.parent();
            }
        }
    }
    return QModelIndex();
}

void NodeSelectorTreeViewWidget::onGenMEGALinkClicked(const QList<mega::MegaHandle>& handles)
{
    mNodeActions.exportLinks(handles);
}
