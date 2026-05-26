#include "NodeSelectorTreeViewWidget.h"

#include "EventUpdater.h"
#include "MegaApplication.h"
#include "MegaNodeNames.h"
#include "NodeSelectorDelegates.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorProxyModel.h"
#include "NodeSelectorSelectionCoordinator.h"
#include "NodeSelectorTreeView.h"
#include "NodeSelectorTreeViewWidgetSpecializations.h"
#include "RequestListenerManager.h"
#include "TokenizableItems/TokenPropertySetter.h"
#include "ui_NodeSelectorTreeViewWidget.h"

#include <QKeyEvent>
#include <QLayout>

#include <algorithm>

const int CHECK_UPDATED_NODES_INTERVAL = 1000;
const int IMMEDIATE_CHECK_UPDATES_NODES_THRESHOLD = 200;

NodeSelectorTreeViewWidget::NodeSelectorTreeViewWidget(SelectTypeSPtr mode,
                                                       TabItem tabType,
                                                       QWidget* parent):
    QWidget(parent),
    ui(new Ui::NodeSelectorTreeViewWidget),
    mProxyModel(nullptr),
    mModel(nullptr),
    mNodeActions(MegaSyncApp->getMegaApi()),
    mMegaApi(MegaSyncApp->getMegaApi()),
    mSelectType(mode),
    mManuallyResizedColumn(false),
    mResizeEventsReceived(0),
    first(true),
    mUiBlocked(false),
    mWasEmpty(true),
    mTabType(tabType)
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

    // Set search tabs titles
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
    ui->emptyPage->setFocusPolicy(Qt::StrongFocus);
    ui->emptyPage->setAcceptDrops(false);

    auto emitUpload = [this]()
    {
        emit onCustomButtonClicked(SelectType::ButtonId::UPLOAD);
    };
    auto emitNewFolder = [this]()
    {
        emit onCustomButtonClicked(SelectType::ButtonId::NEW_FOLDER);
    };
    connect(ui->bEmptyUpload, &QPushButton::clicked, this, emitUpload);
    connect(ui->bEmptyNewFolder, &QPushButton::clicked, this, emitNewFolder);

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
    mSelectType->initTreeViewWidget(this);
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
            &NodeSelectorModelUpdateCoordinator::indexRemovedAffectingCurrentPath,
            this,
            &NodeSelectorTreeViewWidget::onRemovedIndexAffectsCurrentRoot);
    connect(mModelUpdateCoordinator.get(),
            &NodeSelectorModelUpdateCoordinator::viewStateChanged,
            this,
            &NodeSelectorTreeViewWidget::notifyViewStateChanged);

    mNodeActions.setModel(mModel.get());

    enableDragAndDrop(mSelectType->acceptDrops(mTabType));

    ui->tMegaFolders->setSortingEnabled(true);
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

void NodeSelectorTreeViewWidget::initEmptyRootPageMessages()
{
    showRootEmptyState();
}

void NodeSelectorTreeViewWidget::initEmptyFolderMessages()
{
    showFolderEmptyState();
}

void NodeSelectorTreeViewWidget::showRootEmptyState()
{
    applyEmptyState(getEmptyRootPageInfo(), ViewType::ROOT_EMPTY);
}

void NodeSelectorTreeViewWidget::showFolderEmptyState()
{
    if (mSelectType)
    {
        applyEmptyState(mSelectType->getEmptyFolderPageInfo(), ViewType::FOLDER_EMPTY);
    }
}

void NodeSelectorTreeViewWidget::setCurrentPage(ViewType type)
{
    const auto previousType = mCurrentViewType;

    // We still don´t know if the view is empty as it is still loading it
    if ((type == ViewType::ROOT_EMPTY || type == ViewType::FOLDER_EMPTY) &&
        ui->tMegaFolders->loadingView().isLoadingViewSet())
    {
        return;
    }

    switch (type)
    {
        case ViewType::ROOT_EMPTY:
        {
            showRootEmptyState();
            ui->stackedWidget->setCurrentWidget(ui->emptyPage);
            break;
        }
        case ViewType::FOLDER_EMPTY:
        {
            showFolderEmptyState();
            ui->stackedWidget->setCurrentWidget(ui->emptyPage);
            break;
        }
        case ViewType::VIEW:
        default:
        {
            mCurrentViewType = ViewType::VIEW;
            ui->stackedWidget->setCurrentWidget(ui->treeViewPage);
            break;
        }
    }

    if (ui->stackedWidget->currentWidget() != ui->treeViewPage)
    {
        ui->stackedWidget->currentWidget()->setFocus(Qt::OtherFocusReason);
    }
    else
    {
        ui->tMegaFolders->setFocus(Qt::OtherFocusReason);
    }

    if (previousType != mCurrentViewType)
    {
        emit currentViewPageChanged(mCurrentViewType);
    }
}

void NodeSelectorTreeViewWidget::applyEmptyState(const SelectType::EmptyPageInfo& info,
                                                 ViewType type)
{
    mCurrentViewType = type;
    const bool isFolderState = type == ViewType::FOLDER_EMPTY;

    ui->descriptionEmptyLabel->hide();
    ui->titleEmptyLabel->hide();

    if (!info.description.isEmpty())
    {
        ui->descriptionEmptyLabel->setText(info.description);
        ui->descriptionEmptyLabel->show();
    }
    if (!info.title.isEmpty())
    {
        ui->titleEmptyLabel->setText(info.title);
        ui->titleEmptyLabel->show();
    }
    if (!info.icon.isNull())
    {
        ui->emptyIcon->setIcon(info.icon);
    }

    static const char* HAS_BORDER_PROPERTY("hasBorder");
    ui->frame->setProperty(HAS_BORDER_PROPERTY, info.hasBorder);
    ui->verticalLayout_3->setContentsMargins(isFolderState ? 20 : 0,
                                             isFolderState ? 16 : 0,
                                             isFolderState ? 20 : 0,
                                             isFolderState ? 20 : 0);
    ui->gridLayout->setContentsMargins(isFolderState ? 32 : 0,
                                       isFolderState ? 32 : 0,
                                       isFolderState ? 32 : 0,
                                       isFolderState ? 32 : 0);
    ui->gridLayout->setRowStretch(0, isFolderState ? 1 : 1);
    ui->gridLayout->setRowStretch(4, isFolderState ? 1 : 4);
    setStyleSheet(styleSheet());
    setEmptyStateButtonsVisibility(info);
}

void NodeSelectorTreeViewWidget::setEmptyStateButtonsVisibility(
    const SelectType::EmptyPageInfo& info)
{
    const bool showUpload = mSelectType && info.buttons.testFlag(SelectType::ButtonId::UPLOAD) &&
                            mSelectType->showEmptyStateUploadButton(this);
    const bool showNewFolder = mSelectType &&
                               info.buttons.testFlag(SelectType::ButtonId::NEW_FOLDER) &&
                               mSelectType->showEmptyStateNewFolderButton(this);

    ui->bEmptyUpload->setVisible(showUpload);
    ui->bEmptyNewFolder->setVisible(showNewFolder);
    ui->emptyPageButtonsWidget->setVisible(showUpload || showNewFolder);
}

bool NodeSelectorTreeViewWidget::event(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);

        if (mCurrentViewType == ViewType::FOLDER_EMPTY)
        {
            showFolderEmptyState();
        }
        else if (mCurrentViewType == ViewType::ROOT_EMPTY)
        {
            showRootEmptyState();
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
    else if (watched == ui->emptyPage &&
             (event->type() == QEvent::ShortcutOverride || event->type() == QEvent::KeyPress))
    {
        if (auto keyEvent = static_cast<QKeyEvent*>(event); keyEvent->matches(QKeySequence::Paste))
        {
            // Our way to
            if (mSelectType->areActionsAllowed())
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
            }

            return true;
        }
    }
    else if (watched == ui->emptyPage && event->type() == QEvent::ContextMenu)
    {
        ui->tMegaFolders->contextMenuEvent(static_cast<QContextMenuEvent*>(event));
    }

    return QWidget::eventFilter(watched, event);
}

bool NodeSelectorTreeViewWidget::clearSelection()
{
    auto hasSelection{!ui->tMegaFolders->selectedRows().isEmpty()};
    ui->tMegaFolders->clearSelection();
    return hasSelection;
}

void NodeSelectorTreeViewWidget::abort()
{
    mModel->abort();
}

void NodeSelectorTreeViewWidget::moveToTopRootIndex()
{
    setRootIndex(mProxyModel->getTopRootIndex());
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

QModelIndex NodeSelectorTreeViewWidget::getCurrentRootIndex() const
{
    return ui->tMegaFolders->rootIndex();
}

NodeSelectorProxyModel* NodeSelectorTreeViewWidget::getProxyModel()
{
    return mProxyModel.get();
}

QStringList NodeSelectorTreeViewWidget::navigationBreadcrumbSegments() const
{
    if (!mProxyModel)
    {
        return {};
    }

    const auto currentRoot = getCurrentRootIndex();
    if (!currentRoot.isValid())
    {
        return QStringList() << getRootText();
    }

    QStringList segments;
    for (QModelIndex index = currentRoot; index.isValid(); index = index.parent())
    {
        segments.prepend(index.data(Qt::DisplayRole).toString());
    }

    if (!mProxyModel->getTopRootIndex().isValid())
    {
        segments.prepend(getRootText());
    }

    return segments;
}

bool NodeSelectorTreeViewWidget::navigateToBreadcrumbSegment(int segmentIndex)
{
    if (mUiBlocked)
    {
        return false;
    }

    const auto targetIndex = indexForBreadcrumbSegment(segmentIndex);
    const auto currentRoot = getCurrentRootIndex();

    if (targetIndex == currentRoot || (!targetIndex.isValid() && !currentRoot.isValid()))
    {
        return false;
    }

    setRootIndex(targetIndex);
    onSelectionHasChanged();
    return true;
}

bool NodeSelectorTreeViewWidget::isShowingEmptyPage() const
{
    return ui->stackedWidget->currentWidget() == ui->emptyPage;
}

NodeSelectorTreeViewWidget::ViewType NodeSelectorTreeViewWidget::currentViewPage() const
{
    return mCurrentViewType;
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
    ui->emptyPage->setAcceptDrops(enable);
    ui->tMegaFolders->setDragEnabled(enable);
    ui->tMegaFolders->viewport()->setAcceptDrops(enable);
    ui->tMegaFolders->setDropIndicatorShown(enable);
    ui->tMegaFolders->setDragDropMode(enable ? QAbstractItemView::DragDrop :
                                               QAbstractItemView::NoDragDrop);
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
    emit viewStateChanged();
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
        ui->tMegaFolders->setItemDelegate(createItemDelegate(ui->tMegaFolders));
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
                    emit onCustomButtonClicked(SelectType::ButtonId::NEW_FOLDER);
                });
        connect(ui->tMegaFolders,
                &NodeSelectorTreeView::uploadClicked,
                this,
                [this]()
                {
                    emit onCustomButtonClicked(SelectType::ButtonId::UPLOAD);
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

        makeViewConnections();

        setRootIndex(mModel->hasTopRootIndex() ? mProxyModel->index(0, 0) : QModelIndex());

        setStyleSheet(styleSheet());

        // View ready to work with it > View init and model loaded
        emit viewReady();
    }

    // Check empty view page and forward/backward navigation buttons
    checkViewOnModelChange();
}

void NodeSelectorTreeViewWidget::onRemovedIndexAffectsCurrentRoot(const QModelIndex& indexToRemove)
{
    if (!indexToRemove.isValid())
    {
        return;
    }

    const auto currentRoot = ui->tMegaFolders->rootIndex();
    if (!currentRoot.isValid())
    {
        return;
    }

    bool removedIndexIsInCurrentPath = false;
    for (QModelIndex index = currentRoot; index.isValid(); index = index.parent())
    {
        if (index == indexToRemove)
        {
            removedIndexIsInCurrentPath = true;
            break;
        }
    }

    if (!removedIndexIsInCurrentPath)
    {
        return;
    }

    const auto parentIndex = indexToRemove.parent();
    setRootIndex(parentIndex.isValid() ?
                     parentIndex :
                     (mModel->hasTopRootIndex() ? mProxyModel->getTopRootIndex() : QModelIndex()));
}

MegaHandle NodeSelectorTreeViewWidget::getHandleByIndex(const QModelIndex& idx) const
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

bool NodeSelectorTreeViewWidget::isDownloadAllowed() const
{
    return mSelectType->isDownloadAllowed();
}

void NodeSelectorTreeViewWidget::onItemDoubleClick(const QModelIndex& index)
{
    if (isDownloadAllowed())
    {
        auto item = mModel->getItemByIndex(index);
        if (!item || !item->getNode())
        {
            return;
        }

        if (item->getNode()->isFile() && !item->isTakenDown())
        {
            MegaSyncApp->downloadACtionClickedWithHandles(QList<mega::MegaHandle>()
                                                          << item->getNode()->getHandle());
            return;
        }
    }

    if (isAllowedToEnterInIndex(index))
    {
        setRootIndex(index);
    }
}

std::shared_ptr<NodeSelectorProxyModel> NodeSelectorTreeViewWidget::createProxyModel()
{
    return mSelectType->createProxyModel();
}

void NodeSelectorTreeViewWidget::setLoadingSceneVisible(bool blockUi)
{
    if (blockUi && ui->stackedWidget->currentWidget() != ui->treeViewPage)
    {
        setCurrentPage(ViewType::VIEW);
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
            setCurrentPage(ViewType::ROOT_EMPTY);
            return;
        }
    }
    setCurrentPage(ViewType::VIEW);
}

void NodeSelectorTreeViewWidget::updateEmptyStateButtonsVisibility()
{
    if (!mSelectType || ui->stackedWidget->currentWidget() != ui->emptyPage)
    {
        return;
    }

    if (mCurrentViewType == ViewType::FOLDER_EMPTY)
    {
        setEmptyStateButtonsVisibility(mSelectType->getEmptyFolderPageInfo());
    }
    else if (mCurrentViewType == ViewType::ROOT_EMPTY)
    {
        setEmptyStateButtonsVisibility(getEmptyRootPageInfo());
    }
}

QModelIndex NodeSelectorTreeViewWidget::indexForBreadcrumbSegment(int segmentIndex) const
{
    if (!mProxyModel || segmentIndex < 0)
    {
        return QModelIndex();
    }

    QList<QModelIndex> path;
    for (QModelIndex index = getCurrentRootIndex(); index.isValid(); index = index.parent())
    {
        path.prepend(index);
    }

    if (!mProxyModel->getTopRootIndex().isValid())
    {
        if (segmentIndex == 0)
        {
            return QModelIndex();
        }

        --segmentIndex;
    }

    return segmentIndex < path.size() ? path.at(segmentIndex) : QModelIndex();
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
    emit modelModified();

    const bool nowEmpty = (mProxyModel->rowCount(getCurrentRootIndex()) == 0);
    if (nowEmpty != mWasEmpty)
    {
        mWasEmpty = nowEmpty;
        setEmptyFolderPage();
        emit viewStateChanged();
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

QList<MegaHandle> NodeSelectorTreeViewWidget::getMultiSelectionNodeHandle()
{
    auto selectedRows(ui->tMegaFolders->selectedRows());
    return ui->tMegaFolders->getMultiSelectionNodeHandle(selectedRows);
}

QModelIndexList NodeSelectorTreeViewWidget::getSelectedIndexes() const
{
    QModelIndexList selectedIndexes{ui->tMegaFolders->selectedRows()};

    if (selectedIndexes.isEmpty() && !isCurrentRootIndexReadOnly() &&
        mSelectType->isCurrentFolderValidForSelection())
    {
        auto rootIndex = getCurrentRootIndex();
        if (rootIndex.isValid())
        {
            selectedIndexes.append(rootIndex);
        }
    }

    return selectedIndexes;
}

MegaHandle NodeSelectorTreeViewWidget::getSelectedNodeHandle() const
{
    auto selectedCurrentIndex{getSelectedIndexes()};
    return !selectedCurrentIndex.isEmpty() ? getHandleByIndex(selectedCurrentIndex.first()) :
                                             mega::INVALID_HANDLE;
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
    if (auto selectionModel = ui->tMegaFolders->selectionModel())
    {
        selectionModel->clearSelection();

        auto currentIndex = ui->tMegaFolders->rootIndex();
        if (mProxyModel->rowCount(currentIndex) > 0)
        {
            currentIndex = mProxyModel->index(0, NodeSelectorModel::Column::NODE, currentIndex);
        }

        selectionModel->setCurrentIndex(currentIndex, QItemSelectionModel::NoUpdate);
    }

    onRootIndexChanged(node_column_idx);
    setEmptyFolderPage();
    notifyViewStateChanged();
}

void NodeSelectorTreeViewWidget::setEmptyFolderPage()
{
    auto currentRootIndex(getCurrentRootIndex());
    auto topRootIndex(mProxyModel->getTopRootIndex());

    // If we are inside a folder, show the "Empty folder" page.
    if ((currentRootIndex != topRootIndex) && mProxyModel->rowCount(currentRootIndex) == 0)
    {
        if (ui && ui->tMegaFolders->loadingView().isLoadingViewSet())
        {
            return;
        }

        setCurrentPage(ViewType::FOLDER_EMPTY);
    }
    else
    {
        setViewPage();
    }
}

SelectType::EmptyPageInfo NodeSelectorTreeViewWidget::getEmptyRootPageInfo()
{
    return SelectType::EmptyPageInfo();
}

NodeSelectorDelegate* NodeSelectorTreeViewWidget::createItemDelegate(QObject* parent)
{
    return new NodeRowDelegate(parent);
}

void NodeSelectorTreeViewWidget::setColumnHidden(int column, bool hidden)
{
    if (ui->tMegaFolders->isColumnHidden(column) == hidden)
    {
        return;
    }
    ui->tMegaFolders->setColumnHidden(column, hidden);
    updateColumnsWidth(true);
}

void NodeSelectorTreeViewWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    updateColumnsWidth(true);
}

void NodeSelectorTreeViewWidget::setNonInteractiveColumns(const QSet<int>& columns)
{
    if (auto header = qobject_cast<NodeSelectorHeaderView*>(ui->tMegaFolders->header()))
    {
        header->setNonInteractiveSections(columns);
    }
}

QModelIndex NodeSelectorTreeViewWidget::getParentIncomingShareByIndex(QModelIndex idx) const
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
