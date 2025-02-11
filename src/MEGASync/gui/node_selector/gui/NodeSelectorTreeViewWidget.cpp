#include "NodeSelectorTreeViewWidget.h"

#include "DialogOpener.h"
#include "EventUpdater.h"
#include "MegaApplication.h"
#include "MegaNodeNames.h"
#include "NewFolderDialog.h"
#include "NodeSelectorDelegates.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorProxyModel.h"
#include "NodeSelectorTreeViewWidgetSpecializations.h"
#include "QMegaMessageBox.h"
#include "RenameNodeDialog.h"
#include "RequestListenerManager.h"
#include "ui_NodeSelectorTreeViewWidget.h"

const int NodeSelectorTreeViewWidget::LOADING_VIEW_THRESSHOLD = 500;
const int NodeSelectorTreeViewWidget::LABEL_ELIDE_MARGIN = 250;
const char* NodeSelectorTreeViewWidget::FULL_NAME_PROPERTY = "full_name";
const int CHECK_UPDATED_NODES_INTERVAL = 1000;
const int IMMEDIATE_CHECK_UPDATES_NODES_THRESHOLD = 200;

NodeSelectorTreeViewWidget::NodeSelectorTreeViewWidget(SelectTypeSPtr mode, QWidget* parent):
    QWidget(parent),
    ui(new Ui::NodeSelectorTreeViewWidget),
    mProxyModel(nullptr),
    mModel(nullptr),
    mMegaApi(MegaSyncApp->getMegaApi()),
    mManuallyResizedColumn(false),
    first(true),
    mUiBlocked(false),
    mSelectType(mode),
    mNewFolderHandle(mega::INVALID_HANDLE),
    mNewFolderAdded(false)
{
    ui->setupUi(this);
    setFocusProxy(ui->tMegaFolders);
    ui->cbAlwaysUploadToLocation->hide();
    ui->bOk->setDefault(true);
    ui->bOk->setEnabled(false);
    ui->searchButtonsWidget->setVisible(false);
    ui->searchingText->setVisible(false);

    connect(ui->bNewFolder, &QPushButton::clicked, this, &NodeSelectorTreeViewWidget::onbNewFolderClicked);
    connect(ui->bOk, &QPushButton::clicked, this, &NodeSelectorTreeViewWidget::okBtnClicked);
    connect(ui->bCancel, &QPushButton::clicked, this, &NodeSelectorTreeViewWidget::cancelBtnClicked);
    connect(ui->cbAlwaysUploadToLocation, &QCheckBox::stateChanged, this, &NodeSelectorTreeViewWidget::oncbAlwaysUploadToLocationChanged);
    connect(ui->leSearch, &SearchLineEdit::search, this, &NodeSelectorTreeViewWidget::onSearch);

    auto sizePolicy = ui->bNewFolder->sizePolicy();
    sizePolicy.setRetainSizeWhenHidden(true);
    ui->bNewFolder->setSizePolicy(sizePolicy);

    checkBackForwardButtons();
    checkOkCancelButtonsVisibility();
    addCustomBottomButtons(this);

    connect(&ui->tMegaFolders->loadingView(), &ViewLoadingSceneBase::sceneVisibilityChange, this, &NodeSelectorTreeViewWidget::onUiBlocked);

    foreach(auto& button, ui->searchButtonsWidget->findChildren<QAbstractButton*>())
    {
        button->setProperty(ButtonIconManager::CHANGE_LATER, true);
        mButtonIconManager.addButton(button);
    }
}

NodeSelectorTreeViewWidget::~NodeSelectorTreeViewWidget()
{
    delete ui;
}

void NodeSelectorTreeViewWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        if(!ui->tMegaFolders->rootIndex().isValid())
        {
            ui->lFolderName->setText(getRootText());
        }
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}

bool NodeSelectorTreeViewWidget::eventFilter(QObject* watched, QEvent* event)
{
    if(event->type() == QEvent::Drop)
    {
        if(auto dropEvent = static_cast<QDropEvent*>(event))
        {
            if (mModel->dropMimeData(dropEvent->mimeData(),
                                     Qt::MoveAction,
                                     -1,
                                     -1,
                                     mModel->index(0, 0, QModelIndex())))
            {
                dropEvent->acceptProposedAction();
            }
        }
    }
    else if(event->type() == QEvent::DragEnter)
    {
        if(auto dropEvent = static_cast<QDragEnterEvent*>(event))
        {
            if (mModel->canDropMimeData(dropEvent->mimeData(),
                                        Qt::MoveAction,
                                        -1,
                                        -1,
                                        mModel->index(0, 0, QModelIndex())))
            {
                dropEvent->acceptProposedAction();
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

void NodeSelectorTreeViewWidget::init()
{
    mProxyModel = createProxyModel();
    mModel = createModel();
    ui->emptyIcon->setIcon(getEmptyIcon());
    ui->emptyPage->installEventFilter(this);
    mSelectType->init(this);

    ui->tMegaFolders->setSortingEnabled(true);
    mProxyModel->setSourceModel(mModel.get());

    connect(mProxyModel.get(), &NodeSelectorProxyModel::expandReady, this, &NodeSelectorTreeViewWidget::onExpandReady);
    connect(mProxyModel.get(),
            &QAbstractItemModel::rowsInserted,
            this,
            &NodeSelectorTreeViewWidget::onProxyModelRowsInserted);
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
    connect(mModel.get(), &NodeSelectorModel::dataChanged, this, &NodeSelectorTreeViewWidget::onModelDataChanged);
    connect(mModel.get(),
            &NodeSelectorModel::itemsMoved,
            this,
            &NodeSelectorTreeViewWidget::onItemsMoved);

#ifdef __APPLE__
    ui->tMegaFolders->setAnimated(false);
#endif

    connect(&mNodesUpdateTimer,
            &QTimer::timeout,
            this,
            &NodeSelectorTreeViewWidget::processCachedNodesUpdated);
    mNodesUpdateTimer.start(CHECK_UPDATED_NODES_INTERVAL);
}

void NodeSelectorTreeViewWidget::showDefaultUploadOption(bool show)
{
    ui->cbAlwaysUploadToLocation->setVisible(show);
}

void NodeSelectorTreeViewWidget::setSearchText(const QString &text)
{
    ui->leSearch->setText(text);
}

void NodeSelectorTreeViewWidget::setTitleText(const QString &nodeName)
{
    ui->lFolderName->setProperty(FULL_NAME_PROPERTY, nodeName);

    QFontMetrics fm = ui->lFolderName->fontMetrics();

    QString elidedText = fm.elidedText(nodeName, Qt::ElideMiddle, ui->tMegaFolders->width() - LABEL_ELIDE_MARGIN);
    ui->lFolderName->setText(elidedText);

    if(elidedText != nodeName)
        ui->lFolderName->setToolTip(nodeName);
    else
        ui->lFolderName->setToolTip(QString());

}

void NodeSelectorTreeViewWidget::clearSearchText()
{
    ui->leSearch->onClearClicked();
}

void NodeSelectorTreeViewWidget::clearSelection()
{
    ui->tMegaFolders->clearSelection();
}

void NodeSelectorTreeViewWidget::abort()
{
    mModel->abort();
}

NodeSelectorModelItem* NodeSelectorTreeViewWidget::rootItem()
{
    auto rootIndex = ui->tMegaFolders->rootIndex();
    if(!rootIndex.isValid())
    {
        //Top parent
        rootIndex = mModel->index(0,0,QModelIndex());
    }

    return mModel->getItemByIndex(rootIndex);
}

NodeSelectorProxyModel* NodeSelectorTreeViewWidget::getProxyModel()
{
    return mProxyModel.get();
}

bool NodeSelectorTreeViewWidget::isInRootView() const
{
    return !ui->tMegaFolders->rootIndex().isValid();
}

QModelIndex NodeSelectorTreeViewWidget::findIndexToMoveItem()
{
    return mProxyModel->mapFromSource(ui->tMegaFolders->findIndexToMoveItem());
}

void NodeSelectorTreeViewWidget::updateLoadingMessage(std::shared_ptr<MessageInfo> message)
{
    ui->tMegaFolders->getLoadingMessageHandler()->updateMessage(message);
}

void NodeSelectorTreeViewWidget::enableDragAndDrop(bool enable)
{
    ui->tMegaFolders->setDragEnabled(enable);
    ui->tMegaFolders->viewport()->setAcceptDrops(enable);
    ui->tMegaFolders->setDropIndicatorShown(enable);
    ui->tMegaFolders->setDragDropMode(
        enable ? QAbstractItemView::DragDrop : QAbstractItemView::NoDragDrop);
}

void NodeSelectorTreeViewWidget::setDefaultUploadOption(bool value)
{
    ui->cbAlwaysUploadToLocation->setChecked(value);
}

bool NodeSelectorTreeViewWidget::getDefaultUploadOption()
{
    return ui->cbAlwaysUploadToLocation->isChecked();
}
void NodeSelectorTreeViewWidget::setTitle(const QString &title)
{
    ui->lFolderName->setText(title);
}

void NodeSelectorTreeViewWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::BackButton && ui->bBack->isEnabled())
    {
       onGoBackClicked();
    }
    else if(event->button() == Qt::ForwardButton && ui->bForward->isEnabled())
    {
       onGoForwardClicked();
    }
}

void NodeSelectorTreeViewWidget::showEvent(QShowEvent* )
{
    if(!mManuallyResizedColumn)
    {
        ui->tMegaFolders->setColumnWidth(NodeSelectorModel::COLUMN::NODE, qRound(ui->stackedWidget->width() * 0.50));
    }
}

void NodeSelectorTreeViewWidget::resizeEvent(QResizeEvent *)
{
    if(!mManuallyResizedColumn)
    {
        ui->tMegaFolders->setColumnWidth(NodeSelectorModel::COLUMN::NODE, qRound(ui->stackedWidget->width() * 0.50));
    }

    setTitleText(ui->lFolderName->property(FULL_NAME_PROPERTY).toString());
}

void NodeSelectorTreeViewWidget::onSectionResized()
{
    if(!mManuallyResizedColumn
            && ui->tMegaFolders->header()->rect().contains(ui->tMegaFolders->mapFromGlobal(QCursor::pos())))
    {
        mManuallyResizedColumn = true;
    }
}

void NodeSelectorTreeViewWidget::checkViewOnModelChange()
{
    checkBackForwardButtons();
    modelLoaded();
}

void NodeSelectorTreeViewWidget::onProxyModelRowsInserted(const QModelIndex& parent,
                                                          int first,
                                                          int last)
{
    if (mNewFolderAdded)
    {
        for (int index = first; index <= last; ++index)
        {
            auto proxyIndex(mProxyModel->index(index, 0, parent));
            if (proxyIndex.isValid())
            {
                auto handle = getHandleByIndex(proxyIndex);
                // If the row inserted is the new row, stop iterating over the new insertions
                if (handle == mNewFolderHandle)
                {
                    onItemDoubleClick(mProxyModel->getIndexFromHandle(mNewFolderHandle));
                    mNewFolderHandle = mega::INVALID_HANDLE;
                    break;
                }
            }
        }
    }
}

void NodeSelectorTreeViewWidget::onExpandReady()
{
    if(ui->tMegaFolders->model() == nullptr)
    {
        ui->tMegaFolders->setContextMenuPolicy(Qt::DefaultContextMenu);
        ui->tMegaFolders->setExpandsOnDoubleClick(false);
        ui->tMegaFolders->setHeader(new NodeSelectorTreeViewHeaderView(Qt::Horizontal));
        ui->tMegaFolders->setItemDelegate(new NodeRowDelegate(ui->tMegaFolders));
        ui->tMegaFolders->setItemDelegateForColumn(NodeSelectorModel::STATUS, new IconDelegate(ui->tMegaFolders));
        ui->tMegaFolders->setItemDelegateForColumn(NodeSelectorModel::USER, new IconDelegate(ui->tMegaFolders));
        ui->tMegaFolders->setItemDelegateForColumn(NodeSelectorModel::DATE, new DateColumnDelegate(ui->tMegaFolders));
        ui->tMegaFolders->setTextElideMode(Qt::ElideMiddle);

        ui->tMegaFolders->sortByColumn(NodeSelectorModel::NODE, Qt::AscendingOrder);
        ui->tMegaFolders->setModel(mProxyModel.get());

        ui->tMegaFolders->header()->setFixedHeight(NodeSelectorModel::ROW_HEIGHT);
        ui->tMegaFolders->header()->moveSection(NodeSelectorModel::STATUS, NodeSelectorModel::NODE);
        ui->tMegaFolders->setColumnWidth(NodeSelectorModel::COLUMN::STATUS, NodeSelectorModel::ROW_HEIGHT * 2);
        ui->tMegaFolders->header()->setProperty("HeaderIconCenter", true);
        showEvent(nullptr);

        //those connects needs to be done after the model is set, do not move them

        connect(ui->tMegaFolders->selectionModel(), &QItemSelectionModel::selectionChanged, this, &NodeSelectorTreeViewWidget::onSelectionChanged);
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
        connect(ui->tMegaFolders, &NodeSelectorTreeView::getMegaLinkClicked, this, &NodeSelectorTreeViewWidget::onGenMEGALinkClicked);
        connect(ui->tMegaFolders, &QTreeView::doubleClicked, this, &NodeSelectorTreeViewWidget::onItemDoubleClick);
        connect(ui->tMegaFolders, &NodeSelectorTreeView::nodeSelected, this, &NodeSelectorTreeViewWidget::okBtnClicked);
        connect(ui->bForward, &QPushButton::clicked, this, &NodeSelectorTreeViewWidget::onGoForwardClicked);
        connect(ui->bBack, &QPushButton::clicked, this, &NodeSelectorTreeViewWidget::onGoBackClicked);
        connect(ui->tMegaFolders->header(), &QHeaderView::sectionResized, this, &NodeSelectorTreeViewWidget::onSectionResized);

        makeCustomConnections();

        setRootIndex(QModelIndex());
        checkButtonsVisibility();
    }

    auto& indexesToBeExpanded = mModel->needsToBeExpanded();
    if (!indexesToBeExpanded.isEmpty())
    {
        foreach(auto item, indexesToBeExpanded)
        {
            QModelIndex proxyIndex;
            auto handle(item.first);

            if(handle != mega::INVALID_HANDLE)
            {
                proxyIndex = mProxyModel->getIndexFromHandle(handle);
            }

            if (proxyIndex.isValid())
            {
                ui->tMegaFolders->setExpanded(proxyIndex, true);
                indexesToBeExpanded.removeOne(item);
            }
        }
    }

    auto& indexesToBeSelected = mModel->needsToBeSelected();
    if (!indexesToBeSelected.isEmpty())
    {
        foreach(auto item, indexesToBeSelected)
        {
            QModelIndex proxyIndex;
            auto handle(item.first);

            if (handle != mega::INVALID_HANDLE)
            {
                proxyIndex = mProxyModel->getIndexFromHandle(handle);
            }

            if (proxyIndex.isValid())
            {
                selectIndex(proxyIndex, true, false);
                indexesToBeSelected.removeOne(item);
            }
        }
    }
}

void NodeSelectorTreeViewWidget::onGoBackClicked()
{
    auto rootIndex(ui->tMegaFolders->rootIndex());
    auto rootIndexHandle(getHandleByIndex(rootIndex));
    if(rootIndexHandle != mega::INVALID_HANDLE)
    {
        mNavigationInfo.appendToForward(rootIndexHandle);
    }
    QModelIndex indexToGo = getIndexFromHandle(mNavigationInfo.backwardHandles.last());

    setRootIndex(indexToGo);
    checkBackForwardButtons();
    checkButtonsVisibility();

    if(rootIndex.isValid())
    {
        selectIndex(rootIndex, true);
    }
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
                setRootIndex(QModelIndex());
                mNavigationInfo.backwardHandles.clear();
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
            auto handlePos(mNavigationInfo.backwardHandles.indexOf(indexHandleToRemove));
            if (handlePos >= 0)
            {
                changeRootIndex(indexToRemove);
            }
        }
    }
}

void NodeSelectorTreeViewWidget::onGoForwardClicked()
{
    mNavigationInfo.appendToBackward(getHandleByIndex(ui->tMegaFolders->rootIndex()));
    QModelIndex indexToGo = getIndexFromHandle(mNavigationInfo.forwardHandles.last());
    mNavigationInfo.forwardHandles.removeLast();
    setRootIndex(indexToGo);
    checkBackForwardButtons();
    checkButtonsVisibility();

    if(auto selectionModel = ui->tMegaFolders->selectionModel())
    {
        selectionHasChanged(selectionModel->selectedRows());
    }
}

MegaHandle NodeSelectorTreeViewWidget::getHandleByIndex(const QModelIndex& idx)
{
    return mProxyModel ? mProxyModel->getHandle(idx) : mega::INVALID_HANDLE;
}

QModelIndex NodeSelectorTreeViewWidget::getIndexFromHandle(const mega::MegaHandle &handle)
{
    return mProxyModel ? mProxyModel->getIndexFromHandle(handle) : QModelIndex();
}

QModelIndex NodeSelectorTreeViewWidget::getRootIndexFromIndex(const QModelIndex &index)
{
    QModelIndex parentIndex(index);
    while(parentIndex.parent().isValid())
    {
        parentIndex = parentIndex.parent();
    }
    return parentIndex;
}

void NodeSelectorTreeViewWidget::onbNewFolderClicked()
{
    auto parentNode = mProxyModel->getNode(ui->tMegaFolders->rootIndex());
    if (!parentNode)
    {
        parentNode = MegaSyncApp->getRootNode();
        if (!parentNode)
            return;
    }

    QPointer<NewFolderDialog> dialog(new NewFolderDialog(parentNode, ui->tMegaFolders));
    dialog->init();
    DialogOpener::showDialog(dialog,  [this, dialog]()
    {
        auto newNode = dialog->getNewNode();
        //IF the dialog return a node, there are two scenarios:
        //1) The dialog has been accepted, a new folder has been created
        //2) The dialog has been rejected because the folder already exists. If so, select the existing folder
        if(newNode)
        {
            mNewFolderHandle = newNode->getHandle();
            mNewFolderAdded = true;
#ifdef Q_OS_LINUX
            //It seems that the NodeSelector is not activated when the NewFolderDialog is closed,
            //so the ui->tMegaFolders is not correctly focused
            qApp->setActiveWindow(parentWidget()->parentWidget());
#endif

            //Set the focus to the view to allow the user to press enter (or go back, in a future feature)
            ui->tMegaFolders->setFocus();
        }
    });
}

void NodeSelectorTreeViewWidget::oncbAlwaysUploadToLocationChanged(bool value)
{
    foreach(auto& child, parent()->children())
    {
        if(auto tvw = qobject_cast<NodeSelectorTreeViewWidget*>(child))
        {
            if(tvw != sender())
            {
                tvw->setDefaultUploadOption(value);
            }
        }
    }
}

bool NodeSelectorTreeViewWidget::isAllowedToEnterInIndex(const QModelIndex &idx)
{
    return mSelectType->isAllowedToNavigateInside(idx);
}

void NodeSelectorTreeViewWidget::onItemDoubleClick(const QModelIndex &index)
{
    if(!isAllowedToEnterInIndex(index))
    {
        auto item = mModel->getItemByIndex(index);
        if(item && item->getNode()->isFile())
        {
            MegaSyncApp->downloadACtionClickedWithHandles(QList<mega::MegaHandle>() << item->getNode()->getHandle());
        }
        return;
    }

    mNavigationInfo.appendToBackward(getHandleByIndex(ui->tMegaFolders->rootIndex()));
    mNavigationInfo.removeFromForward(mProxyModel->getHandle(index));

    setRootIndex(index);
    checkBackForwardButtons();
    checkButtonsVisibility();
}

void NodeSelectorTreeViewWidget::checkButtonsVisibility()
{
    mSelectType->newFolderButtonVisibility(this);
    mSelectType->customButtonsVisibility(this);
}

void NodeSelectorTreeViewWidget::checkOkCancelButtonsVisibility()
{
    mSelectType->okCancelButtonsVisibility(this);
}

void NodeSelectorTreeViewWidget::addCustomBottomButtons(NodeSelectorTreeViewWidget *wdg)
{
    auto buttonsMap = mSelectType->addCustomBottomButtons(wdg);
    foreach(auto id, buttonsMap.keys())
    {
        auto button = buttonsMap.value(id);
        if(button)
        {
            ui->customBottomButtonsLayout->addWidget(button);
            connect(button,
                    &QPushButton::clicked,
                    this,
                    [this, id]()
                    {
                        emit onCustomBottomButtonClicked(id);
                    });
        }
    }
}

std::unique_ptr<NodeSelectorProxyModel> NodeSelectorTreeViewWidget::createProxyModel()
{
    return std::unique_ptr<NodeSelectorProxyModel>(new NodeSelectorProxyModel);
}

void NodeSelectorTreeViewWidget::setLoadingSceneVisible(bool blockUi)
{
    ui->tMegaFolders->loadingView().toggleLoadingScene(blockUi);

    if(!blockUi)
    {
        modelLoaded();
    }
}

void NodeSelectorTreeViewWidget::modelLoaded()
{
    if(mModel)
    {
        if(mModel->rowCount() == 0 && showEmptyView())
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
    if(mUiBlocked != state)
    {
        mUiBlocked = state;

        ui->bNewFolder->setDisabled(state);
        ui->bCancel->setDisabled(state);
        ui->searchButtonsWidget->setDisabled(state);

        if(!state)
        {
            if(auto selectionModel = ui->tMegaFolders->selectionModel())
            {
                selectionHasChanged(selectionModel->selectedRows());
            }
            checkBackForwardButtons();
        }
        else
        {
            ui->bBack->setEnabled(false);
            ui->bForward->setEnabled(false);
            ui->bOk->setDisabled(true);
        }

        processCachedNodesUpdated();
    }
}

void NodeSelectorTreeViewWidget::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(deselected)
    Q_UNUSED(selected)

    if(!mUiBlocked)
    {
        if(auto selectionModel = ui->tMegaFolders->selectionModel())
        {
            selectionHasChanged(selectionModel->selectedRows());
        }
    }
}

void NodeSelectorTreeViewWidget::onModelDataChanged(const QModelIndex &first, const QModelIndex &last, const QVector<int> &roles)
{
    if(ui->tMegaFolders->selectionModel())
    {
        auto selectedRows(ui->tMegaFolders->selectionModel()->selectedRows());
        if(selectedRows.contains(mProxyModel->mapFromSource(first)))
        {
            //Update the buttons visibility/enable dependant on the selection
            selectionHasChanged(selectedRows);
        }
    }
}

void NodeSelectorTreeViewWidget::selectionHasChanged(const QModelIndexList &selected)
{
    ui->bOk->setEnabled(mSelectType->okButtonEnabled(this, selected));
    mSelectType->selectionHasChanged(selected, this);
}

void NodeSelectorTreeViewWidget::onRenameClicked()
{
    auto node = std::unique_ptr<MegaNode>(mMegaApi->getNodeByHandle(getSelectedNodeHandle()));
    int access = mMegaApi->getAccess(node.get());
    //This is for an extra protection as we don´t show the rename action if one of this conditions are not met
    if (!node || access < MegaShare::ACCESS_FULL  || !node->isNodeKeyDecrypted())
    {
        return;
    }

    QPointer<RenameRemoteNodeDialog> dialog(new RenameRemoteNodeDialog(std::move(node), this));
    dialog->init();
    DialogOpener::showDialog(dialog);
}

void NodeSelectorTreeViewWidget::onDeleteClicked(const QList<mega::MegaHandle> &handles, bool permanently)
{
    if (handles.isEmpty())
    {
        return;
    }

    auto getNode = [this](mega::MegaHandle handle) -> std::shared_ptr<mega::MegaNode>
    {
        auto node = std::shared_ptr<MegaNode>(mMegaApi->getNodeByHandle(handle));

        //This is for an extra protection as we don´t show the rename action if oxne of this conditions are not met
        if (!node || !node->isNodeKeyDecrypted())
        {
            return nullptr;
        }

        return node;
    };

    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.parent = ui->tMegaFolders;
    msgInfo.title = MegaSyncApp->getMEGAString();
    msgInfo.buttons = QMessageBox::Yes | QMessageBox::No;
    msgInfo.defaultButton = QMessageBox::Yes;
    msgInfo.buttonsText.insert(QMessageBox::Yes, tr("Delete"));
    msgInfo.buttonsText.insert(QMessageBox::No, tr("Cancel"));
    msgInfo.finishFunc = [this, handles, permanently](QPointer<QMessageBox> msg)
    {
        if (msg->result() == QMessageBox::Yes)
        {
            mModel->deleteNodes(handles, permanently);
        }
        else
        {
            return;
        }
    };

    if (permanently)
    {
        msgInfo.informativeText = tr("You cannot undo this action");
    }
    else
    {
        msgInfo.informativeText =
            tr("Any shared files or folders will no longer be accessible to the people you shared "
               "them with. You can still access these items in the Rubbish bin, restore, and share "
               "them.");
    }

    auto type(Utilities::getHandlesType(handles));

    if (permanently)
    {
        if (type == Utilities::HandlesType::FILES)
        {
            msgInfo.text =
                tr("You are about to permanently delete %n file. Would you like to proceed?",
                   "",
                   handles.size());
        }
        else if (type == Utilities::HandlesType::FOLDERS)
        {
            msgInfo.text =
                tr("You are about to permanently delete %n folder. Would you like to proceed?",
                   "",
                   handles.size());
        }
        else
        {
            msgInfo.text =
                tr("You are about to permanently delete %1 items. Would you like to proceed?")
                    .arg(handles.size());
        }
    }
    else
    {
        if (handles.size() == 1)
        {
            auto node = getNode(handles.first());
            if (node)
            {
                msgInfo.text = tr("Are you sure that you want to delete \"%1\"?")
                                   .arg(QString::fromUtf8(node->getName()));
            }
        }
        else
        {
            if (type == Utilities::HandlesType::FILES)
            {
                msgInfo.text =
                    tr("Are you sure that you want to delete %1 file?").arg(handles.size());
            }
            else if (type == Utilities::HandlesType::FOLDERS)
            {
                msgInfo.text =
                    tr("Are you sure that you want to delete %1 folder?").arg(handles.size());
            }
            else
            {
                msgInfo.text =
                    tr("You are about to permanently delete %1 items. Would you like to proceed?")
                        .arg(handles.size());
            }
        }
    }

    QMegaMessageBox::warning(msgInfo);
}

void NodeSelectorTreeViewWidget::onLeaveShareClicked(const QList<mega::MegaHandle>& handles)
{
    if (handles.isEmpty())
    {
        return;
    }

    auto getNode = [this](mega::MegaHandle handle) -> std::shared_ptr<mega::MegaNode>
    {
        auto node = std::shared_ptr<MegaNode>(mMegaApi->getNodeByHandle(handle));

        // This is for an extra protection as we don´t show the rename action if oxne of this
        // conditions are not met
        if (!node || !node->isNodeKeyDecrypted())
        {
            return nullptr;
        }

        return node;
    };

    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.parent = ui->tMegaFolders;
    msgInfo.title = MegaSyncApp->getMEGAString();
    msgInfo.buttons = QMessageBox::Yes | QMessageBox::No;
    msgInfo.defaultButton = QMessageBox::Yes;
    msgInfo.buttonsText.insert(QMessageBox::Yes, tr("Leave"));
    msgInfo.buttonsText.insert(QMessageBox::No, tr("Don’t leave"));

    msgInfo.informativeText =
        tr("You will leave the inshared folder. You will stop having access to the folder.");
    if (handles.size() == 1)
    {
        auto node = getNode(handles.first());
        if (node)
        {
            msgInfo.text = tr("You are about to leave \"%1\".\nWould you like to proceed?")
                               .arg(QString::fromUtf8(node->getName()));
        }
    }
    else
    {
        msgInfo.text =
            tr("You are about to leave %n folder.\nWould you like to proceed?", "", handles.size());
    }

    msgInfo.finishFunc = [this, handles](QPointer<QMessageBox> msg)
    {
        if (msg->result() == QMessageBox::Yes)
        {
            mModel->deleteNodes(handles, true);
        }
        else
        {
            return;
        }
    };
    QMegaMessageBox::warning(msgInfo);
}

NodeSelectorTreeViewWidget::NodeState
    NodeSelectorTreeViewWidget::getNodeOnModelState(mega::MegaNode* node)
{
    NodeState result(NodeState::DOESNT_EXIST);

    auto findIndex = [this](mega::MegaHandle handle, const QModelIndex& parent = QModelIndex())
    {
        return mModel->findIndexByNodeHandle(handle, parent);
    };

    if (node)
    {
        auto parentHandle(node->getParentHandle());

        if (parentHandle != mega::INVALID_HANDLE)
        {
            auto currentIndex(findIndex(node->getHandle()));
            auto parentIndex = findIndex(parentHandle);

            if (parentIndex.isValid())
            {
                auto parentItem = mModel->getItemByIndex(parentIndex);
                if (parentItem->areChildrenInitialized())
                {
                    if (currentIndex.parent() == parentIndex)
                    {
                        result = NodeState::EXISTS;
                    }
                    else if (currentIndex.isValid())
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
                    if (currentIndex.isValid())
                    {
                        result = NodeState::MOVED_OUT_OF_VIEW;
                    }
                    else
                    {
                        result = NodeState::EXISTS_BUT_INVISIBLE;
                    }
                }
            }
            else if (currentIndex.isValid())
            {
                result = NodeState::REMOVE;
            }
            else if (isNodeCompatibleWithModel(node))
            {
                result = NodeState::EXISTS_BUT_INVISIBLE;
            }
        }
    }

    return result;
}

bool NodeSelectorTreeViewWidget::onNodesUpdate(mega::MegaApi*, mega::MegaNodeList *nodes)
{
    if(!nodes)
    {
        return false;
    }

    QList<UpdateNodesInfo> updatedNodes;

    for (int i = 0; i < nodes->size(); i++)
    {
        MegaNode* node = nodes->get(i);

        if(mModel->rootNodeUpdated(node))
        {
            continue;
        }

        if(node->getParentHandle() != mega::INVALID_HANDLE)
        {
            if (node->getChanges() & (MegaNode::CHANGE_TYPE_PARENT |
                                      MegaNode::CHANGE_TYPE_NEW))
            {
                std::unique_ptr<mega::MegaNode> parentNode(MegaSyncApp->getMegaApi()->getNodeByHandle(node->getParentHandle()));

                auto existenceType(getNodeOnModelState(node));

                if (existenceType == NodeState::REMOVE)
                {
                    mRemovedNodes.insert(node->getHandle());
                }
                else if (existenceType != NodeState::DOESNT_EXIST)
                {
                    std::unique_ptr<mega::MegaNode> parentNode(MegaSyncApp->getMegaApi()->getNodeByHandle(node->getParentHandle()));
                    if(parentNode)
                    {
                        //Check if the node exists or if we need to add it
                        if (existenceType == NodeState::ADD || existenceType == NodeState::MOVED)
                        {
                            UpdateNodesInfo info;
                            info.parentHandle = parentNode->getHandle();
                            info.previousHandle = node->getHandle();
                            info.node = std::shared_ptr<mega::MegaNode>(node->copy());
                            updatedNodes.append(info);

                            if (existenceType == NodeState::MOVED)
                            {
                                mRemoveMovedNodes.insert(node->getHandle());
                            }
                        }
                        else if (existenceType == NodeState::EXISTS_BUT_INVISIBLE ||
                                 existenceType == NodeState::MOVED_OUT_OF_VIEW)
                        {
                            mUpdatedButInvisibleNodes.append(node->getHandle());

                            if (existenceType == NodeState::MOVED_OUT_OF_VIEW)
                            {
                                mRemoveMovedNodes.insert(node->getHandle());
                            }
                        }
                    }
                }
            }
            else if(node->getChanges() & MegaNode::CHANGE_TYPE_NAME)
            {
                if (getNodeOnModelState(node) == NodeState::EXISTS)
                {
                    UpdateNodesInfo info;
                    info.previousHandle = node->getHandle();
                    info.node = std::shared_ptr<mega::MegaNode>(node->copy());
                    mRenamedNodesByHandle.append(info);
                }
            }
            //Moved or new version added
            else if(node->getChanges() & MegaNode::CHANGE_TYPE_REMOVED)
            {
                if (getNodeOnModelState(node) == NodeState::EXISTS)
                {
                    mRemovedNodes.insert(node->getHandle());
                }
            }
        }
    }

    foreach(auto updateNode, updatedNodes)
    {
        if (!updateNode.node->isFile() || mModel->showFiles())
        {
            mAddedNodesByParentHandle.insert(updateNode.parentHandle, updateNode.node);
        }
    }

    if(areThereNodesToUpdate())
    {
        if(shouldUpdateImmediately())
        {
            if(mNodesUpdateTimer.interval() != 0)
            {
                mNodesUpdateTimer.setInterval(0);
            }
        }
        else if(mNodesUpdateTimer.interval() != CHECK_UPDATED_NODES_INTERVAL)
        {
            mNodesUpdateTimer.setInterval(CHECK_UPDATED_NODES_INTERVAL);
        }

        return true;
    }
    else
    {
        return false;
    }
}

bool NodeSelectorTreeViewWidget::shouldUpdateImmediately()
{
    int totalSize = mUpdatedNodesByPreviousHandle.size();
    if(totalSize > IMMEDIATE_CHECK_UPDATES_NODES_THRESHOLD)
    {
        return true;
    }
    totalSize += mRemovedNodes.size();
    if(totalSize > IMMEDIATE_CHECK_UPDATES_NODES_THRESHOLD)
    {
        return true;
    }
    totalSize += mRemoveMovedNodes.size();
    if (totalSize > IMMEDIATE_CHECK_UPDATES_NODES_THRESHOLD)
    {
        return true;
    }
    totalSize += mRenamedNodesByHandle.size();
    if(totalSize > IMMEDIATE_CHECK_UPDATES_NODES_THRESHOLD)
    {
        return true;
    }
    totalSize += mAddedNodesByParentHandle.size();
    if(totalSize > IMMEDIATE_CHECK_UPDATES_NODES_THRESHOLD)
    {
        return true;
    }
    totalSize += mUpdatedButInvisibleNodes.size();
    if (totalSize > IMMEDIATE_CHECK_UPDATES_NODES_THRESHOLD)
    {
        return true;
    }
    return false;
}

bool NodeSelectorTreeViewWidget::areThereNodesToUpdate()
{
    return !mUpdatedNodesByPreviousHandle.isEmpty() || !mRemovedNodes.isEmpty() ||
           !mRenamedNodesByHandle.isEmpty() || !mAddedNodesByParentHandle.isEmpty() ||
           !mRemoveMovedNodes.isEmpty() || !mUpdatedButInvisibleNodes.isEmpty();
}

void NodeSelectorTreeViewWidget::selectIndex(const mega::MegaHandle& handle,
    bool setCurrent,
    bool exclusiveSelect)
{
    auto index(mProxyModel->getIndexFromHandle(handle));
    if(index.isValid())
    {
        selectIndex(index, setCurrent, exclusiveSelect);
    }
}

void NodeSelectorTreeViewWidget::initMovingNodes(int number)
{
    mModel->initMovingNodes(number);
}

bool NodeSelectorTreeViewWidget::increaseMovingNodes()
{
    return mModel->increaseMovingNodes();
}

bool NodeSelectorTreeViewWidget::areItemsAboutToBeMovedFromHere(mega::MegaHandle firstHandleMoved,
                                                                int handlesMoved)
{
    // Check if the move and drop is in the same model
    if (!mModel->isMovingNodes())
    {
        auto itemIndex(mModel->findIndexByNodeHandle(firstHandleMoved, QModelIndex()));
        if (itemIndex.isValid())
        {
            initMovingNodes(handlesMoved);
            return true;
        }
    }

    return false;
}

void NodeSelectorTreeViewWidget::selectIndex(const QModelIndex& index,
                                             bool setCurrent,
                                             bool exclusiveSelect)
{
    auto selectionFlag(exclusiveSelect ? QItemSelectionModel::ClearAndSelect :
                                         QItemSelectionModel::Select);

    if(setCurrent)
    {
        ui->tMegaFolders->selectionModel()->setCurrentIndex(index,
                                                            selectionFlag |
                                                                QItemSelectionModel::Rows);
    }
    ui->tMegaFolders->selectionModel()->select(index, selectionFlag | QItemSelectionModel::Rows);
    ui->tMegaFolders->scrollTo(index, QAbstractItemView::ScrollHint::PositionAtCenter);
}

void NodeSelectorTreeViewWidget::onItemsMoved()
{
    if (!mMovedHandlesToSelect.isEmpty())
    {
        clearSelection();

        setSelectedNodeHandle(mMovedHandlesToSelect.takeFirst());
        mModel->selectIndexesByHandleAsync(mMovedHandlesToSelect);

        mMovedHandlesToSelect.clear();
    }
}

void NodeSelectorTreeViewWidget::removeItemByHandle(mega::MegaHandle handle)
{
    auto index = mModel->findIndexByNodeHandle(handle, QModelIndex());
    if(index.isValid())
    {
        auto proxyIndex(mProxyModel->mapFromSource(index));
        if(proxyIndex.isValid())
        {
            onRemoveIndexFromGoBack(proxyIndex);

            if (mNavigationInfo.forwardHandles.contains(handle))
            {
                mNavigationInfo.forwardHandles.removeLast();
            }

            checkBackForwardButtons();

            mProxyModel->deleteNode(proxyIndex);
            mNavigationInfo.remove(handle);
        }
    }
}

void NodeSelectorTreeViewWidget::processCachedNodesUpdated()
{
    //We check if the model is being modified (insert rows, remove rows...etc) before each action in order to avoid
    //calling twice to begininsertrows (as some of these actions are performed in different threads...)
    if(!mProxyModel->isModelProcessing() && !mModel->isRequestingNodes() && areThereNodesToUpdate())
    {
        if(!mModel->isBeingModified())
        {
            foreach(auto info, mRenamedNodesByHandle)
            {
                updateNode(info, true);
            }
            mRenamedNodesByHandle.clear();
        }

        if(!mModel->isBeingModified())
        {
            foreach(auto info, mUpdatedNodesByPreviousHandle)
            {
                updateNode(info, false);
            }
            mUpdatedNodesByPreviousHandle.clear();
        }

        if(!mModel->isBeingModified())
        {
            foreach(auto handle, mRemovedNodes)
            {
                removeItemByHandle(handle);
                mModel->moveProcessed();
            }
            mRemovedNodes.clear();
        }

        if(!mModel->isBeingModified())
        {
            foreach(auto handle, mRemoveMovedNodes)
            {
                removeItemByHandle(handle);
            }
            mRemoveMovedNodes.clear();
        }

        if (!mModel->isBeingModified())
        {
            for (int index = 0; index < mUpdatedButInvisibleNodes.size(); ++index)
            {
                auto handle(mUpdatedButInvisibleNodes.at(index));

                // Just in case
                removeItemByHandle(handle);
                mMovedHandlesToSelect.append(handle);

                mModel->moveProcessed();
            }

            mUpdatedButInvisibleNodes.clear();
        }

        if(!mModel->isBeingModified())
        {
            foreach(auto& parentHandle, mAddedNodesByParentHandle.uniqueKeys())
            {
                auto parentIndex = getAddedNodeParent(parentHandle);
                auto addedNodes(mAddedNodesByParentHandle.values(parentHandle));
                auto finalNodes(addedNodes);

                mModel->addNodes(finalNodes, parentIndex);

                for (auto& node: qAsConst(finalNodes))
                {
                    if (!MegaSyncApp->getMegaApi()->isInRubbish(node.get()))
                    {
                        mMovedHandlesToSelect.append(node->getHandle());
                    }
                }

                //Only for root indexes
                auto proxyParentIndex(mProxyModel->mapFromSource(parentIndex));
                if(!proxyParentIndex.parent().isValid())
                {
                    ui->tMegaFolders->setExpanded(proxyParentIndex, true);
                }
            }

            mAddedNodesByParentHandle.clear();
        }
    }
}

void NodeSelectorTreeViewWidget::updateNode(const UpdateNodesInfo &info, bool scrollTo)
{
    auto index = mModel->findIndexByNodeHandle(info.previousHandle, QModelIndex());
    auto proxyIndex = mProxyModel->mapFromSource(index);

    auto isSelected(false);

    if(scrollTo)
    {
        if(ui->tMegaFolders->selectionModel())
        {
            if(proxyIndex.isValid())
            {
                isSelected = ui->tMegaFolders->selectionModel()->isSelected(proxyIndex);
            }
        }
    }

    mModel->moveProcessed();
    mModel->updateItemNode(index, info.node);

    if(info.node)
    {
        if(proxyIndex.isValid() &&
           ui->tMegaFolders->rootIndex() == proxyIndex)
        {
            setTitleText(MegaNodeNames::getNodeName(info.node.get()));
        }
    }

    if(isSelected)
    {
        //The proxy index may has changed,, update it
        proxyIndex = mProxyModel->mapFromSource(index);
        ui->tMegaFolders->scrollTo(proxyIndex, QAbstractItemView::ScrollHint::PositionAtCenter);
    }
}

void NodeSelectorTreeViewWidget::setSelectedNodeHandle(const MegaHandle& selectedHandle)
{
    if(selectedHandle == INVALID_HANDLE)
    {
        return;
    }

    auto node = std::shared_ptr<MegaNode>(mMegaApi->getNodeByHandle(selectedHandle));
    if (!node)
        return;

    mProxyModel->setExpandMapped(true);

    mModel->loadTreeFromNode(node);
}

MegaHandle NodeSelectorTreeViewWidget::getSelectedNodeHandle()
{
    return ui->tMegaFolders->getSelectedNodeHandle();
}

QList<MegaHandle> NodeSelectorTreeViewWidget::getMultiSelectionNodeHandle()
{
    return ui->tMegaFolders->getMultiSelectionNodeHandle();
}

QModelIndex NodeSelectorTreeViewWidget::getSelectedIndex()
{
    QModelIndex ret;
    if(ui->tMegaFolders->selectionModel()->selectedRows().size() > 0)
        ret = ui->tMegaFolders->selectionModel()->selectedRows().at(0);
    return ret;
}

void NodeSelectorTreeViewWidget::checkBackForwardButtons()
{
    ui->bBack->setEnabled(!mNavigationInfo.backwardHandles.isEmpty());
    ui->bForward->setEnabled(!mNavigationInfo.forwardHandles.isEmpty());
}

void NodeSelectorTreeViewWidget::setRootIndex(const QModelIndex &proxy_idx)
{
    // Everytime we move among folders, we reset the selection
    ui->tMegaFolders->selectionModel()->clear();

    //In case the idx is coming from a potentially hidden column, we always take the NODE column
    //As it is the only one that have childrens
    auto node_column_idx = proxy_idx.sibling(proxy_idx.row(), NodeSelectorModel::COLUMN::NODE);

    mModel->setCurrentRootIndex(mProxyModel->mapToSource(node_column_idx));
    ui->tMegaFolders->setRootIndex(node_column_idx);

    // Remove in case the rootindex is in the backward list
    auto indexHandleToRemove(getHandleByIndex(node_column_idx));
    auto handlePos(mNavigationInfo.backwardHandles.indexOf(indexHandleToRemove));
    if (handlePos >= 0)
    {
        while (mNavigationInfo.backwardHandles.last() != indexHandleToRemove)
        {
            mNavigationInfo.backwardHandles.removeLast();
        }

        mNavigationInfo.backwardHandles.removeLast();
    }

    onRootIndexChanged(node_column_idx);

    if(!node_column_idx.isValid())
    {
        setTitleText(getRootText());

        ui->lOwnerIcon->setPixmap(QPixmap());
        ui->avatarSpacer->spacerItem()->changeSize(0, 0);
        ui->lIcon->setPixmap(QPixmap());
        ui->syncSpacer->spacerItem()->changeSize(0, 0);
        return;
    }

    //Taking the sync icon
    auto status_column_idx = proxy_idx.sibling(proxy_idx.row(), NodeSelectorModel::COLUMN::STATUS);
    QIcon syncIcon = qvariant_cast<QIcon>(status_column_idx.data(Qt::DecorationRole));

    if(!syncIcon.isNull())
    {
        QPixmap pm = syncIcon.pixmap(QSize(NodeSelectorModelItem::ICON_SIZE, NodeSelectorModelItem::ICON_SIZE), QIcon::Normal);
        ui->lIcon->setPixmap(pm);
        ui->syncSpacer->spacerItem()->changeSize(10, 0);
    }
    else
    {
        ui->lIcon->setPixmap(QPixmap());
        ui->syncSpacer->spacerItem()->changeSize(0, 0);
    }

    auto item = NodeSelectorModel::getItemByIndex(node_column_idx);
    if(!item)
    {
        return;
    }

    auto node = item->getNode();
    if(node)
    {
        setTitleText(MegaNodeNames::getNodeName(node.get()));
    }
}

QIcon NodeSelectorTreeViewWidget::getEmptyIcon()
{
    return QIcon();
}

QModelIndex NodeSelectorTreeViewWidget::getParentIncomingShareByIndex(QModelIndex idx)
{
    while(idx.isValid())
    {
        if(auto item = NodeSelectorModel::getItemByIndex(idx))
        {
            if(item->getNode()->isInShare())
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


void NodeSelectorTreeViewWidget::onGenMEGALinkClicked()
{
    auto node = std::unique_ptr<MegaNode>(mMegaApi->getNodeByHandle(getSelectedNodeHandle()));
    if (!node || node->getType() == MegaNode::TYPE_ROOT
            || mMegaApi->getAccess(node.get()) != MegaShare::ACCESS_OWNER)
    {
        return;
    }
    mMegaApi->exportNode(node.get());
}

void NodeSelectorTreeViewWidget::Navigation::removeFromForward(const mega::MegaHandle &handle)
{
    if(forwardHandles.isEmpty())
        return;

    auto megaApi = MegaSyncApp->getMegaApi();
    auto p_node = std::unique_ptr<mega::MegaNode>(megaApi->getNodeByHandle(handle));

    QMap<MegaHandle, MegaHandle> parentHandles;
    while(p_node)
    {
        MegaHandle actualHandle = p_node->getHandle();
        p_node.reset(megaApi->getParentNode(p_node.get()));
        MegaHandle parentHandle= INVALID_HANDLE;
        if(p_node)
            parentHandle = p_node->getHandle();
        parentHandles.insert(parentHandle, actualHandle);
    }

    p_node.reset(megaApi->getNodeByHandle(forwardHandles.last()));
    QMap<MegaHandle, MegaHandle> actualListParentHandles;
    while(p_node)
    {
        MegaHandle actualHandle = p_node->getHandle();
        p_node.reset(megaApi->getParentNode(p_node.get()));
        MegaHandle parentHandle= INVALID_HANDLE;
        if(p_node)
            parentHandle = p_node->getHandle();
        actualListParentHandles.insert(parentHandle, actualHandle);
    }

    for(auto it = actualListParentHandles.begin(); it != actualListParentHandles.end(); ++it)
    {
        if(parentHandles.contains(it.key()))
        {
            forwardHandles.clear();
            return;
        }
    }
}

void NodeSelectorTreeViewWidget::Navigation::remove(const mega::MegaHandle &handle)
{
    backwardHandles.removeAll(handle);
    int forwardPos = forwardHandles.indexOf(handle);
    for(int i = 0; i <= forwardPos; i++)
    {
        forwardHandles.removeFirst();
    }
}

void NodeSelectorTreeViewWidget::Navigation::appendToBackward(const mega::MegaHandle &handle)
{
    if(!backwardHandles.contains(handle))
        backwardHandles.append(handle);
}

void NodeSelectorTreeViewWidget::Navigation::appendToForward(const mega::MegaHandle &handle)
{
    if(!forwardHandles.contains(handle))
        forwardHandles.append(handle);
}

void NodeSelectorTreeViewWidget::Navigation::clear()
{
    backwardHandles.clear();
    forwardHandles.clear();
}

bool SelectType::isAllowedToNavigateInside(const QModelIndex &index)
{
    auto item = NodeSelectorModel::getItemByIndex(index);
    if(!item)
    {
        return false;
    }
    return !(item->getNode()->isFile() || item->isCloudDrive() || item->isRubbishBin());
}

bool SelectType::cloudDriveIsCurrentRootIndex(NodeSelectorTreeViewWidget* wdg)
{
    auto result(false);
    auto rootItem = wdg->rootItem();
    if(rootItem)
    {
        result = rootItem->getNode() &&
                 (rootItem->getNode()->getHandle() == MegaSyncApp->getRootNode()->getHandle());
    }

    return result;
}

void DownloadType::init(NodeSelectorTreeViewWidget *wdg)
{
    wdg->ui->bNewFolder->hide();
    wdg->ui->tMegaFolders->setSelectionMode(QAbstractItemView::ExtendedSelection);
    wdg->mModel->showFiles(true);
    wdg->mModel->showReadOnlyFolders(true);
}

bool DownloadType::okButtonEnabled(NodeSelectorTreeViewWidget* wdg, const QModelIndexList &selected)
{
    auto result(!selected.isEmpty());
    if(selected.isEmpty())
    {
        result = !cloudDriveIsCurrentRootIndex(wdg);
    }

    return result;
}

NodeSelectorModelItemSearch::Types DownloadType::allowedTypes()
{
    return NodeSelectorModelItemSearch::Type::CLOUD_DRIVE
            | NodeSelectorModelItemSearch::Type::INCOMING_SHARE
            | NodeSelectorModelItemSearch::Type::BACKUP;
}

void SyncType::init(NodeSelectorTreeViewWidget *wdg)
{
    wdg->mModel->setSyncSetupMode(true);
    wdg->ui->bNewFolder->setVisible(wdg->newFolderBtnCanBeVisisble());
    wdg->mModel->showFiles(false);
    wdg->mModel->showReadOnlyFolders(false);
}

void SyncType::newFolderButtonVisibility(NodeSelectorTreeViewWidget *wdg)
{
    if(wdg->newFolderBtnCanBeVisisble())
    {
        auto sourceIndex = wdg->mProxyModel->getIndexFromSource(wdg->ui->tMegaFolders->rootIndex());
        wdg->ui->bNewFolder->setVisible(sourceIndex.isValid() || !wdg->isCurrentRootIndexReadOnly());
    }
}

bool SyncType::okButtonEnabled(NodeSelectorTreeViewWidget*, const QModelIndexList &selected)
{
    bool enable(false);
    if(!selected.isEmpty() && selected.size() < 2)
    {
        auto& index = selected.at(0);
        bool isSyncable = index.data(toInt(NodeSelectorModelRoles::IS_SYNCABLE_FOLDER_ROLE)).toBool();
        bool isFile = index.data(toInt(NodeSelectorModelRoles::IS_FILE_ROLE)).toBool();
        if(isSyncable && !isFile)
        {
            enable = true;
        }
    }
    return enable;
}

NodeSelectorModelItemSearch::Types SyncType::allowedTypes()
{
    return NodeSelectorModelItemSearch::Type::CLOUD_DRIVE | NodeSelectorModelItemSearch::Type::INCOMING_SHARE;
}

bool SyncType::isAllowedToNavigateInside(const QModelIndex& index)
{
    if(!SelectType::isAllowedToNavigateInside(index))
    {
        return false;
    }
    auto item = NodeSelectorModel::getItemByIndex(index);
    return !(item->getStatus() == NodeSelectorModelItem::Status::SYNC || item->getStatus() == NodeSelectorModelItem::Status::SYNC_CHILD);
}

void StreamType::init(NodeSelectorTreeViewWidget *wdg)
{
    wdg->ui->bNewFolder->hide();
    wdg->mModel->showFiles(true);
    wdg->mModel->showReadOnlyFolders(true);
}

bool StreamType::okButtonEnabled(NodeSelectorTreeViewWidget*, const QModelIndexList &selected)
{
    bool enable(false);
    if(!selected.isEmpty() && selected.size() < 2)
    {
        auto& index = selected.at(0);
        enable = index.data(toInt(NodeSelectorModelRoles::IS_FILE_ROLE)).toBool();
    }
    return enable;
}

NodeSelectorModelItemSearch::Types StreamType::allowedTypes()
{
    return NodeSelectorModelItemSearch::Type::CLOUD_DRIVE
            | NodeSelectorModelItemSearch::Type::INCOMING_SHARE
            | NodeSelectorModelItemSearch::Type::BACKUP;
}

void UploadType::init(NodeSelectorTreeViewWidget *wdg)
{
    wdg->ui->bNewFolder->setVisible(wdg->newFolderBtnCanBeVisisble());
    wdg->mModel->showFiles(false);
    wdg->mModel->showReadOnlyFolders(false);
}

void UploadType::newFolderButtonVisibility(NodeSelectorTreeViewWidget *wdg)
{
    if(wdg->newFolderBtnCanBeVisisble())
    {
        wdg->ui->bNewFolder->setVisible(!wdg->isCurrentRootIndexReadOnly());
    }
}

bool UploadType::okButtonEnabled(NodeSelectorTreeViewWidget* wdg, const QModelIndexList &selected)
{
    auto result(!selected.isEmpty());
    if(selected.isEmpty())
    {
        result = !cloudDriveIsCurrentRootIndex(wdg);
    }

    return result;
}

NodeSelectorModelItemSearch::Types UploadType::allowedTypes()
{
    return NodeSelectorModelItemSearch::Type::CLOUD_DRIVE
            | NodeSelectorModelItemSearch::Type::INCOMING_SHARE;
}

//////////////////////////////////////////////////////////////////
void CloudDriveType::init(NodeSelectorTreeViewWidget *wdg)
{
    wdg->ui->bNewFolder->hide();
    wdg->ui->tMegaFolders->setSelectionMode(QAbstractItemView::ExtendedSelection);
    wdg->mModel->showFiles(true);
    wdg->mModel->showReadOnlyFolders(true);
}


bool CloudDriveType::okButtonEnabled(NodeSelectorTreeViewWidget*, const QModelIndexList &selected)
{
    return !selected.isEmpty();
}

NodeSelectorModelItemSearch::Types CloudDriveType::allowedTypes()
{
    return NodeSelectorModelItemSearch::Type::CLOUD_DRIVE |
           NodeSelectorModelItemSearch::Type::INCOMING_SHARE |
           NodeSelectorModelItemSearch::Type::BACKUP | NodeSelectorModelItemSearch::Type::RUBBISH;
}

void CloudDriveType::okCancelButtonsVisibility(NodeSelectorTreeViewWidget *wdg)
{
    wdg->ui->bOk->setVisible(false);
    wdg->ui->bCancel->setVisible(false);
}

void CloudDriveType::newFolderButtonVisibility(NodeSelectorTreeViewWidget *wdg)
{
    wdg->ui->bNewFolder->setVisible(!wdg->isCurrentRootIndexReadOnly() && !wdg->isCurrentSelectionReadOnly());
}

void CloudDriveType::customButtonsVisibility(NodeSelectorTreeViewWidget *wdg)
{
    auto rubbishWidget = dynamic_cast<NodeSelectorTreeViewWidgetRubbish*>(wdg);
    if(rubbishWidget)
    {
        mCustomBottomButtons.value(wdg).value(ButtonId::Upload)->setVisible(false);
        mCustomBottomButtons.value(wdg).value(ButtonId::Download)->setVisible(false);
        mCustomBottomButtons.value(wdg).value(ButtonId::ClearRubbish)->setVisible(!rubbishWidget->isEmpty());
    }
    else
    {
        mCustomBottomButtons.value(wdg).value(ButtonId::ClearRubbish)->setVisible(false);
        mCustomBottomButtons.value(wdg).value(ButtonId::Upload)->setVisible(!wdg->isCurrentSelectionReadOnly());
    }
}

QMap<uint, QPushButton*> CloudDriveType::addCustomBottomButtons(NodeSelectorTreeViewWidget* wdg)
{
    auto& buttons = mCustomBottomButtons[wdg];
    if(buttons.isEmpty())
    {
        auto uploadButton(new QPushButton(QIcon(QString::fromUtf8("://images/transfer_manager/toolbar/upload_toolbar_ico_default.png")), MegaApplication::tr("Upload")));
        buttons.insert(ButtonId::Upload, uploadButton);

        auto downloadButton(new QPushButton(QIcon(QString::fromUtf8("://images/transfer_manager/toolbar/download_toolbar_ico_default.png")), MegaApplication::tr("Download")));
        buttons.insert(ButtonId::Download, downloadButton);

        auto clearRubbishButton(new QPushButton(QIcon(QString::fromUtf8("://images/transfer_manager/sidebar/cancel_all_ico_hover.png")), NodeSelectorTreeViewWidget::tr("Empty Rubbish bin")));
        buttons.insert(ButtonId::ClearRubbish, clearRubbishButton);
        clearRubbishButton->hide();
    }

    return buttons;
}

void CloudDriveType::selectionHasChanged(const QModelIndexList &selected, NodeSelectorTreeViewWidget *wdg)
{    
    auto buttons = mCustomBottomButtons.value(wdg);

    auto rubbishWidget = dynamic_cast<NodeSelectorTreeViewWidgetRubbish*>(wdg);

    if(rubbishWidget)
    {
        buttons.value(ButtonId::Download)->setVisible(false);
        buttons.value(ButtonId::Upload)->setVisible(false);
        buttons.value(ButtonId::ClearRubbish)->setVisible(!rubbishWidget->isEmpty());
    }
    else
    {
        buttons.value(ButtonId::ClearRubbish)->setVisible(false);

        buttons.value(ButtonId::Download)->setVisible(!selected.isEmpty() || !wdg->isInRootView());

        bool uploadEnabled(false);
        if(selected.size() == 1)
        {
            uploadEnabled = !selected.first().data(toInt(NodeSelectorModelRoles::IS_FILE_ROLE)).toBool() && !wdg->isCurrentSelectionReadOnly();
        }
        else if (selected.size() == 0 && !wdg->isCurrentRootIndexReadOnly())
        {
            uploadEnabled = true;
        }
        buttons.value(ButtonId::Upload)->setVisible(uploadEnabled);
    }

}


//////////////////
NodeSelectorModelItemSearch::Types MoveBackupType::allowedTypes()
{
    return NodeSelectorModelItemSearch::Type::CLOUD_DRIVE;
}

