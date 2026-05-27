#include "NodeSelectorSpecializations.h"

#include "DialogOpener.h"
#include "DuplicatedNodeDialog.h"
#include "megaapi.h"
#include "MegaNodeNames.h"
#include "MessageDialogOpener.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorProxyModel.h"
#include "NodeSelectorTreeViewWidgetSpecializations.h"
#include "Preferences.h"
#include "RequestListenerManager.h"
#include "StatsEventHandler.h"
#include "SyncInfo.h"
#include "TabSelector.h"
#include "TextDecorator.h"
#include "tokenizer/TokenizableItems/TokenizableButtons.h"
#include "ui_NodeSelector.h"
#include "UploadToMegaDialog.h"

#include <QBoxLayout>
#include <QMessageBox>
#include <QPointer>
#include <QStyle>

UploadNodeSelector::UploadNodeSelector(QWidget* parent):
    FilePickerNodeSelector(SelectTypeSPtr(new UploadType), parent)
{}

void UploadNodeSelector::onOkButtonClicked()
{
    auto node = getSelectedNode();
    if (node)
    {
        int access = Utilities::getNodeAccess(node->getHandle());
        if (access < mega::MegaShare::ACCESS_READWRITE)
        {
            MessageDialogInfo msgInfo;
            msgInfo.parent = this;
            msgInfo.descriptionText =
                tr("You need Read & Write or Full access rights to be able to "
                   "upload to the selected folder.");
            msgInfo.finishFunc = [this](QPointer<MessageDialogResult>)
            {
                reject();
            };
            MessageDialogOpener::warning(msgInfo);
        }
        else
        {
            accept();
        }
    }
    else
    {
        showNotFoundNodeMessageBox();
    }
}

/////////////////////////////////////////////////////////////
DownloadNodeSelector::DownloadNodeSelector(QWidget* parent):
    FilePickerNodeSelector(SelectTypeSPtr(new DownloadType), parent)
{
    setWindowTitle(tr("Download"));
}

void DownloadNodeSelector::onOkButtonClicked()
{
    QList<mega::MegaHandle> nodes = getMultiSelectionNodeHandle();
    int wrongNodes(0);
    foreach(auto& nodeHandle, nodes)
    {
        auto node = std::unique_ptr<mega::MegaNode>(mMegaApi->getNodeByHandle(nodeHandle));
        if (!node)
        {
            ++wrongNodes;
        }
    }

    MessageDialogInfo msgInfo;
    msgInfo.parent = this;
    msgInfo.finishFunc = [this](QPointer<MessageDialogResult>)
    {
        reject();
    };

    if (wrongNodes == nodes.size())
    {
        if (ui->stackedWidget->currentIndex() == NodeSelectorTreeViewWidget::TabItem::CLOUD_DRIVE)
        {
            msgInfo.descriptionText =
                tr("The item you selected has been removed. To reselect, close "
                   "this window and try again.",
                   "",
                   wrongNodes);
            MessageDialogOpener::warning(msgInfo);
        }
        else
        {
            msgInfo.descriptionText =
                tr("You no longer have access to this item. Ask the owner to share again.",
                   "",
                   wrongNodes);
            MessageDialogOpener::warning(msgInfo);
        }
    }
    else if (wrongNodes > 0)
    {
        QString warningMsg1 = tr("%1 item selected", "", static_cast<int>(nodes.size()))
                                  .arg(static_cast<int>(nodes.size()));
        msgInfo.descriptionText =
            tr("%1. %2 has been removed. To reselect, close this window and try again.",
               "",
               wrongNodes)
                .arg(warningMsg1)
                .arg(wrongNodes);
        MessageDialogOpener::warning(msgInfo);
    }
    else
    {
        accept();
    }
}

/////////////////////////////////////////////////////////////
SyncNodeSelector::SyncNodeSelector(QWidget* parent):
    FilePickerNodeSelector(SelectTypeSPtr(new SyncType), parent)
{
    if (mIncomingSharesWidget)
    {
        connect(mIncomingSharesWidget,
                &NodeSelectorTreeViewWidget::viewStateChanged,
                this,
                &SyncNodeSelector::refreshDestinationBreadcrumb);
    }
}

QString SyncNodeSelector::destinationBreadcrumbEmptyText()
{
    return tr("Select a full access shared folder to sync");
}

void SyncNodeSelector::onModelModified()
{
    // Syncs is the only FilePicker that can show/hide the breadcrumb/banner depending on the model
    refreshDestinationBreadcrumb();
}

void SyncNodeSelector::refreshDestinationBreadcrumb()
{
    // Case 3: SHARES tab with no shares → hide breadcrumb and banner; the tree view
    // shows its own empty state.
    if (incomingSharesTabIsEmpty())
    {
        ui->destinationBanner->setVisible(false);
        ui->destinationBreadcrumb->setVisible(false);
        ui->destinationBreadcrumb->setPathSegments({});
        return;
    }

    FilePickerNodeSelector::refreshDestinationBreadcrumb();
}

bool SyncNodeSelector::incomingSharesTabIsEmpty() const
{
    auto* currentWidget = getCurrentTreeViewWidget();
    if (!currentWidget ||
        currentWidget->getTabType() != NodeSelectorTreeViewWidget::TabItem::SHARES)
    {
        return false;
    }

    auto* proxy = currentWidget->getProxyModel();
    if (!proxy)
    {
        return false;
    }

    return proxy->rowCount(proxy->getTopRootIndex()) == 0;
}

bool SyncNodeSelector::isFullSync()
{
    auto syncsList = SyncInfo::instance()->getSyncSettingsByType(SyncInfo::SyncType::TYPE_TWOWAY);
    auto foundIt =
        std::find_if(syncsList.cbegin(),
                     syncsList.cend(),
                     [](const auto& sync)
                     {
                         return (sync->getMegaFolder() == QLatin1String("/") && sync->isActive());
                     });

    return foundIt != syncsList.cend();
}

QString SyncNodeSelector::destinationTitleText() const
{
    return tr("Folder to sync");
}

FilePickerNodeSelector::DestinationBannerInfo SyncNodeSelector::destinationBannerInfo() const
{
    DestinationBannerInfo info{BannerWidget::Type::BANNER_WARNING, {}};

    auto* contextWidget = getSearchAwareTargetWidget();
    if (!contextWidget)
    {
        return info;
    }

    // While searching, the selection lives in the search widget, not in the source tab
    // returned by getSearchAwareTargetWidget(); consult the currently visible widget.
    auto* selectionWidget = getCurrentTreeViewWidget();
    const auto selectedIndexes =
        selectionWidget ? selectionWidget->getSelectedIndexes() : QModelIndexList();
    if (selectedIndexes.size() == 1)
    {
        const auto status = selectedIndexes.first()
                                .data(toInt(NodeSelectorModelRoles::STATUS_ROLE))
                                .value<NodeSelectorModelItem::Status>();

        switch (status)
        {
            case NodeSelectorModelItem::Status::SYNC:
            case NodeSelectorModelItem::Status::SYNC_CHILD:
            {
                info.text = tr("Choose a different folder. This folder is already synced");
                break;
            }
            case NodeSelectorModelItem::Status::SYNC_PARENT:
            {
                info.text = tr("Choose a different folder. This location contains a folder that's "
                               "already synced");
                break;
            }
            default:
            {
                break;
            }
        }
    }
    else if (contextWidget->getTabType() == NodeSelectorTreeViewWidget::TabItem::SHARES)
    {
        if (!fullAccessInTopRootShares())
        {
            info.text = tr("Only shared folders with full access can be synced");
        }
        else if (!enableFoldersInTopRootShares())
        {
            info.text = tr("Choose a different folder. This location contains a folder that's "
                           "already synced");
        }
    }

    return info;
}

bool SyncNodeSelector::fullAccessInTopRootShares() const
{
    if (!mIncomingSharesWidget)
    {
        return false;
    }

    auto* proxy = mIncomingSharesWidget->getProxyModel();
    if (!proxy)
    {
        return false;
    }

    const auto topRoot = proxy->getTopRootIndex();
    const int rowCount = proxy->rowCount(topRoot);
    if (rowCount == 0)
    {
        return false;
    }

    for (int row = 0; row < rowCount; ++row)
    {
        const auto idx = proxy->index(row, 0, topRoot);
        if (idx.data(toInt(NodeSelectorModelRoles::ACCESS_ROLE)).toInt() ==
            mega::MegaShare::ACCESS_FULL)
        {
            return true;
        }
    }

    return false;
}

bool SyncNodeSelector::enableFoldersInTopRootShares() const
{
    if (!mIncomingSharesWidget)
    {
        return false;
    }

    auto* proxy = mIncomingSharesWidget->getProxyModel();
    if (!proxy)
    {
        return false;
    }

    const auto topRoot = proxy->getTopRootIndex();
    const int rowCount = proxy->rowCount(topRoot);
    if (rowCount == 0)
    {
        return false;
    }

    for (int row = 0; row < rowCount; ++row)
    {
        const auto idx = proxy->index(row, 0, topRoot);
        if (idx.flags() & Qt::ItemIsEnabled)
        {
            return true;
        }
    }

    return false;
}

void SyncNodeSelector::onOkButtonClicked()
{
    auto node = getSelectedNode();
    if (node)
    {
        MessageDialogInfo msgInfo;
        msgInfo.parent = this;
        msgInfo.finishFunc = [this](QPointer<MessageDialogResult>)
        {
            reject();
        };

        int access = Utilities::getNodeAccess(node->getHandle());
        if (access < mega::MegaShare::ACCESS_FULL)
        {
            msgInfo.descriptionText =
                tr("You need Full access right to be able to sync the selected folder.");
            MessageDialogOpener::warning(msgInfo);
        }
        else
        {
            std::unique_ptr<char[]> path(mMegaApi->getNodePath(node.get()));
            auto check = std::unique_ptr<mega::MegaNode>(mMegaApi->getNodeByPath(path.get()));
            if (!check)
            {
                msgInfo.descriptionText =
                    tr("Invalid folder for synchronization.\n"
                       "Please, ensure that you don't use characters like '\\' '/' "
                       "or ':' in your folder names.");
                MessageDialogOpener::warning(msgInfo);
            }
            else
            {
                accept();
            }
        }
    }
    else
    {
        showNotFoundNodeMessageBox();
    }
}

/////////////////////////////////////////////////////////////
StreamNodeSelector::StreamNodeSelector(QWidget* parent):
    FilePickerNodeSelector(SelectTypeSPtr(new StreamType), parent)
{}

QString StreamNodeSelector::destinationTitleText() const
{
    return tr("Select a file to stream");
}

FilePickerNodeSelector::DestinationBannerInfo StreamNodeSelector::destinationBannerInfo() const
{
    auto selectedNode(getSelectedNode());

    if (selectedNode && selectedNode->isFile())
    {
        return {};
    }

    return {BannerWidget::Type::BANNER_MESSAGE, tr("Select a file to stream")};
}

void StreamNodeSelector::onOkButtonClicked()
{
    auto node = getSelectedNode();
    if (node)
    {
        if (node->isFolder())
        {
            MessageDialogInfo msgInfo;
            msgInfo.parent = this;
            msgInfo.descriptionText = tr("Only files can be used for streaming.");
            msgInfo.finishFunc = [this](QPointer<MessageDialogResult>)
            {
                reject();
            };
            MessageDialogOpener::warning(msgInfo);
        }
        else
        {
            accept();
        }
    }
    else
    {
        showNotFoundNodeMessageBox();
    }
}

/////////////////////////////////////////////////////////////
CloudDriveNodeSelector::CloudDriveNodeSelector(QWidget* parent):
    NodeSelector(SelectTypeSPtr(new CloudDriveType), parent)
{
    ui->bOk->hide();
    ui->bCancel->hide();

    mDragBackDrop = new QWidget(this);
    mDragBackDrop->hide();

    ui->destinationBreadcrumb->setVisible(false);

    setAcceptDrops(true);

#ifndef Q_OS_MACOS
    Qt::WindowFlags flags = Qt::Window;
    this->setWindowFlags(flags);
#ifdef Q_OS_LINUX
    this->setWindowFlags(this->windowFlags());
#endif
#endif

    // Update last time opened
    Preferences::instance()->cloudDriveDialogOpened();

    connect(ui->fSearch,
            &TabSelector::clicked,
            this,
            &CloudDriveNodeSelector::onbShowSearchClicked);
    connect(ui->fSearch, &TabSelector::hidden, this, &CloudDriveNodeSelector::onfShowSearchHidden);

    // Send stats in case we didn´t send them in the current hour
    sendStats();
}

void CloudDriveNodeSelector::specialisedTreeViewWidgetsCreated()
{
    NodeSelector::specialisedTreeViewWidgetsCreated();

    if (mSearchWidget)
    {
        connect(mSearchWidget,
                &NodeSelectorTreeViewWidgetSearch::searchTabTypeChanged,
                this,
                &CloudDriveNodeSelector::configureSearchWidget);
    }
}

void CloudDriveNodeSelector::configureCloudDriveWidget()
{
    if (!mCloudDriveWidget)
    {
        return;
    }
    mCloudDriveWidget->setColumnHidden(NodeSelectorModel::Column::USER, true);
    mCloudDriveWidget->setColumnHidden(NodeSelectorModel::Column::ACCESS, true);
}

void CloudDriveNodeSelector::configureIncomingSharesWidget()
{
    if (!mIncomingSharesWidget)
    {
        return;
    }
    const bool insideShare = mIncomingSharesWidget->getCurrentRootIndex().isValid();
    mIncomingSharesWidget->setColumnHidden(NodeSelectorModel::Column::USER, insideShare);
    mIncomingSharesWidget->setColumnHidden(NodeSelectorModel::Column::ACCESS, insideShare);
    mIncomingSharesWidget->setColumnHidden(NodeSelectorModel::Column::ADDED_DATE, true);
}

void CloudDriveNodeSelector::configureBackupsWidget()
{
    if (!mBackupsWidget)
    {
        return;
    }
    mBackupsWidget->setColumnHidden(NodeSelectorModel::Column::USER, true);
    mBackupsWidget->setColumnHidden(NodeSelectorModel::Column::ACCESS, true);
}

void CloudDriveNodeSelector::configureRubbishWidget()
{
    if (!mRubbishWidget)
    {
        return;
    }
    mRubbishWidget->setColumnHidden(NodeSelectorModel::Column::USER, true);
    mRubbishWidget->setColumnHidden(NodeSelectorModel::Column::ACCESS, true);
}

void CloudDriveNodeSelector::configureSearchWidget(TabType type)
{
    if (!mSearchWidget)
    {
        return;
    }

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
        {
            break;
        }
    }

    mSearchWidget->setColumnHidden(NodeSelectorModel::Column::USER, hideUserColumn);
    mSearchWidget->setColumnHidden(NodeSelectorModel::Column::ACCESS, hideAccessColumn);
    mSearchWidget->setColumnHidden(NodeSelectorModel::Column::ADDED_DATE, hideAddedDate);
}

void CloudDriveNodeSelector::configureSidebar()
{
    static constexpr int EXPANDED_SIDEBAR_WIDTH = 256;
    static constexpr int EXPANDED_TAB_HEIGHT = 32;

    ui->wLeftPaneNS->setMinimumWidth(EXPANDED_SIDEBAR_WIDTH);
    ui->wLeftPaneNS->setMaximumWidth(EXPANDED_SIDEBAR_WIDTH);

    const auto expandTab = [](TabSelector* tab)
    {
        if (!tab)
        {
            return;
        }
        tab->setIconOnly(false);
        tab->setProperty("class", QLatin1String("sidebar"));
        tab->setMinimumHeight(EXPANDED_TAB_HEIGHT);
        tab->setMaximumHeight(EXPANDED_TAB_HEIGHT);
        tab->style()->unpolish(tab);
        tab->style()->polish(tab);
    };

    expandTab(ui->fCloudDrive);
    expandTab(ui->fIncomingShares);
    expandTab(ui->fBackups);
    expandTab(ui->fRubbish);
    expandTab(ui->fSearch);

    resize(1024, 720);
    setMinimumSize(1024, 691);
}

void CloudDriveNodeSelector::configureHeader()
{
    // Search Line Edit
    ui->leSearchTool->setMode(SearchLineEdit::Mode::ALWAYS_EXPANDED);

    static constexpr int SEARCH_LINE_EDIT_FIXED_WIDTH = 345;
    static constexpr int SEARCH_LINE_EDIT_FIXED_HEIGHT = 40;

    ui->leSearchTool->setFixedSize(SEARCH_LINE_EDIT_FIXED_WIDTH, SEARCH_LINE_EDIT_FIXED_HEIGHT);
}

void CloudDriveNodeSelector::configureActionButtonsPlacement()
{
    const auto applyHeaderStyle = [](TokenizableButton* btn, const QString& type)
    {
        if (!btn)
        {
            return;
        }
        btn->setProperty("type", type);
        btn->setProperty("dimension", QLatin1String("medium"));
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
    };

    applyHeaderStyle(ui->bUpload, QLatin1String("primary"));
    applyHeaderStyle(ui->bClearRubbish, QLatin1String("primary"));
    applyHeaderStyle(ui->bNewFolder, QLatin1String("secondary"));

    for (auto* btn: {ui->bUpload, ui->bNewFolder, ui->bClearRubbish})
    {
        ui->actionButtonsLayout->addWidget(btn);
    }

    ui->actionButtonsLayout->addStretch();
}

void CloudDriveNodeSelector::configureFooterVisibility()
{
    ui->footer->setVisible(false);
    ui->wRightPaneNS->layout()->setContentsMargins(0, 0, 0, 14);
}

NodeSelector::ClearTypes CloudDriveNodeSelector::searchClearType() const
{
    return ClearType::CLEAR_ON_CLOSE_SEARCH_TAB;
}

void CloudDriveNodeSelector::onLanguageChangeEvent() {}

void CloudDriveNodeSelector::handleSearch(const QString& text)
{
    ui->wSearch->show();
    ui->fSearch->setTitle(text);
    ui->fSearch->setSelected(true);

    mSearchWidget->search(text);
    onbShowSearchClicked();
}

void CloudDriveNodeSelector::handleSearchHidden()
{
    if (!ui->wSearch->isVisible())
    {
        return;
    }

    ui->wSearch->hide();
    ui->fSearch->setTitle(QString());
    ui->leSearchTool->onClearClicked();
    mSearchWidget->stopSearch();

    if (getCurrentTreeViewWidget() == mSearchWidget)
    {
        onbShowCloudDriveClicked();
    }
}

void CloudDriveNodeSelector::sendStats()
{
    auto lastDateTimeStatSent(Preferences::instance()->cloudDriveDialogLastDateTimeStatSent());
    auto lastDateTimeOpened(Preferences::instance()->cloudDriveDialogLastDateTimeOpened());

    // If event sent during the current hour, return
    if (!Utilities::hourHasChanged(lastDateTimeStatSent,
                                   QDateTime::currentDateTime().toSecsSinceEpoch()))
    {
        return;
    }

    auto sendEvent = []()
    {
        MegaSyncApp->getStatsEventHandler()->sendEvent(
            AppStatsEvents::EventType::CLOUD_DRIVE_HOURLY_ACTIVE_USERS);

        Preferences::instance()->updateCloudDriveDialogLastDateTimeStatSent();
    };

    // If the CloudDrive is currently open, send the event
    // If the CloudDrive is not currently open but it was opened during the current hour, send
    // the event
    auto cloudDriveDialog(DialogOpener::findDialog<NodeSelector>());
    auto isCurrentlyOpen(cloudDriveDialog && dynamic_cast<CloudDriveNodeSelector*>(
                                                 cloudDriveDialog->getDialog().data()));

    if (isCurrentlyOpen)
    {
        sendEvent();
    }
    else
    {
        // If the hour between now and the last date time opened has changed, it is not the
        // current hour
        auto openedDuringCurrentHour(
            !(Utilities::hourHasChanged(QDateTime::currentDateTime().toSecsSinceEpoch(),
                                        lastDateTimeOpened)));
        if (openedDuringCurrentHour)
        {
            sendEvent();
        }
    }
}

void CloudDriveNodeSelector::onCustomButtonClicked(uint id)
{
    if (id == SelectType::UPLOAD)
    {
        MegaSyncApp->getStatsEventHandler()->sendTrackedEvent(
            AppStatsEvents::EventType::CLOUD_DRIVE_UPLOAD_CLICKED);
        auto currentTreeWidget = getCurrentTreeViewWidget();
        auto rootItem = currentTreeWidget ? currentTreeWidget->rootItem() : nullptr;
        if (rootItem)
        {
            auto node = rootItem->getNode();
            if (node)
            {
                MegaSyncApp->runUploadActionWithTargetHandle(node->getHandle(),
                                                             mega::MegaApi::PITAG_TRIGGER_PICKER,
                                                             this);
            }
            else
            {
                showNotFoundNodeMessageBox();
            }
        }
    }
    else if (id == SelectType::CLEAR_RUBBISH)
    {
        MessageDialogInfo msgInfo;
        msgInfo.parent = this;
        msgInfo.buttons = QMessageBox::Yes | QMessageBox::No;
        QMap<QMessageBox::Button, QString> textsByButton;
        textsByButton.insert(QMessageBox::Yes, tr("Empty"));
        textsByButton.insert(QMessageBox::No, tr("Cancel"));
        msgInfo.buttonsText = textsByButton;
        msgInfo.defaultButton = QMessageBox::No;
        msgInfo.textFormat = Qt::RichText;
        msgInfo.titleText = tr("Empty Rubbish bin?");
        msgInfo.descriptionText =
            tr("All items will be permanently deleted. This action can [B]not[/B] be undone");
        msgInfo.finishFunc = [this](QPointer<MessageDialogResult> msg)
        {
            if (msg->result() == QMessageBox::Yes)
            {
                mRubbishWidget->setLoadingSceneVisible(true);
                MegaSyncApp->getMegaApi()->cleanRubbishBin();
            }
        };
        MessageDialogOpener::warning(msgInfo);
    }

    NodeSelector::onCustomButtonClicked(id);
}

void CloudDriveNodeSelector::onItemsAboutToBeMoved(const QList<mega::MegaHandle>& handles, int type)
{
    checkMovingItems(handles, type, NodeSelector::IncreaseOrDecrease::INCREASE);
}

void CloudDriveNodeSelector::onItemsAboutToBeMovedFailed(const QList<mega::MegaHandle>& handles,
                                                         int type)
{
    checkMovingItems(handles, type, NodeSelector::IncreaseOrDecrease::DECREASE);
}

void CloudDriveNodeSelector::onItemRequestsFinished(int actionType)
{
    if (actionType == MoveActionType::DELETE_RUBBISH && mRubbishWidget->isUiBlocked())
    {
        mRubbishWidget->finishMovingNodes();
    }
}

void CloudDriveNodeSelector::onItemsAboutToBeRestored(const QSet<mega::MegaHandle>& handles)
{
    auto tabsInfo(getTabs(handles.values()));

    if (!tabsInfo.cloudDriveNodes.isEmpty())
    {
        mCloudDriveWidget->setParentOfRestoredNodes(handles);
    }

    if (!tabsInfo.incomingSharedNodes.isEmpty())
    {
        mIncomingSharesWidget->setParentOfRestoredNodes(handles);
    }
}

void CloudDriveNodeSelector::onItemAboutToBeReplaced(mega::MegaHandle handle)
{
    auto tabsInfo(getTabs(QList<mega::MegaHandle>() << handle));

    if (!tabsInfo.cloudDriveNodes.isEmpty())
    {
        mCloudDriveWidget->addHandleToBeReplaced(handle);
    }

    if (!tabsInfo.incomingSharedNodes.isEmpty())
    {
        mIncomingSharesWidget->addHandleToBeReplaced(handle);
    }
}

void CloudDriveNodeSelector::onItemsAboutToBeMerged(
    const QList<std::shared_ptr<NodeSelectorMergeInfo>>& mergesInfo,
    int actionType)
{
    performMergeAction(mergesInfo, actionType, NodeSelector::IncreaseOrDecrease::INCREASE);
}

void CloudDriveNodeSelector::onItemMergeFinished(mega::MegaHandle sourceHandle,
                                                 mega::MegaHandle targetHandle,
                                                 int)
{
    const auto targetTabsInfo(getTabs(QList<mega::MegaHandle>() << targetHandle));

    auto finishMergeInWidget =
        [sourceHandle, targetHandle](const QList<mega::MegaHandle>& targetHandlesByTab,
                                     NodeSelectorTreeViewWidget* wid)
    {
        if (!wid || !targetHandlesByTab.contains(targetHandle))
        {
            return;
        }

        QMultiHash<NodeSelectorTreeViewWidget::SourceHandle,
                   NodeSelectorTreeViewWidget::TargetHandle>
            merges;
        merges.insert(sourceHandle, targetHandle);

        wid->decreaseMovingNodes(1);
        wid->resetMergeFolderHandles(merges);
    };

    finishMergeInWidget(targetTabsInfo.cloudDriveNodes, mCloudDriveWidget);
    finishMergeInWidget(targetTabsInfo.incomingSharedNodes, mIncomingSharesWidget);
}

void CloudDriveNodeSelector::onItemsAboutToBeMergedFailed(
    const QList<std::shared_ptr<NodeSelectorMergeInfo>>& mergesInfo,
    int actionType)
{
    performMergeAction(mergesInfo, actionType, NodeSelector::IncreaseOrDecrease::DECREASE);
}

void CloudDriveNodeSelector::onbShowSearchClicked()
{
    if (ui->fSearch->isVisible())
    {
        ui->stackedWidget->setCurrentWidget(mSearchWidget);
    }
}

void CloudDriveNodeSelector::onfShowSearchHidden()
{
    handleSearchHidden();
}

void CloudDriveNodeSelector::performMergeAction(
    const QList<std::shared_ptr<NodeSelectorMergeInfo>>& mergesInfo,
    int actionType,
    IncreaseOrDecrease type)
{
    QList<NodeSelectorTreeViewWidget::SourceHandle> sourceHandles;
    QList<NodeSelectorTreeViewWidget::TargetHandle> targetHandles;
    for (const auto& info: mergesInfo)
    {
        sourceHandles.append(info->nodeToMerge->getHandle());
        targetHandles.append(info->nodeTarget->getHandle());
    }

    if (actionType != MoveActionType::COPY)
    {
        performItemsToBeMoved(sourceHandles,
                              NodeSelector::IncreaseOrDecrease::INCREASE,
                              true,
                              false);
    }

    auto fillMergeFolders =
        [type,
         &mergesInfo](const QList<NodeSelectorTreeViewWidget::TargetHandle>& targetHandlesByTab,
                      NodeSelectorTreeViewWidget* wid)
    {
        QMultiHash<NodeSelectorTreeViewWidget::SourceHandle,
                   NodeSelectorTreeViewWidget::TargetHandle>
            merges;
        for (const auto& info: mergesInfo)
        {
            if (targetHandlesByTab.contains(info->nodeTarget->getHandle()))
            {
                merges.insert(info->nodeToMerge->getHandle(), info->nodeTarget->getHandle());
            }
        }

        if (type == NodeSelector::IncreaseOrDecrease::INCREASE)
        {
            wid->increaseMovingNodes(static_cast<int>(merges.size()));
            wid->setMergeFolderHandles(merges);
        }
        else
        {
            wid->decreaseMovingNodes(static_cast<int>(merges.size()));
            wid->resetMergeFolderHandles(merges);
        }
    };

    auto targetTabsInfo(getTabs(targetHandles));

    if (!targetTabsInfo.cloudDriveNodes.isEmpty())
    {
        if (actionType != MoveActionType::COPY)
        {
            fillMergeFolders(targetTabsInfo.cloudDriveNodes, mCloudDriveWidget);
        }
        else
        {
            mCloudDriveWidget->setAsyncSelectedNodeHandle(targetTabsInfo.cloudDriveNodes);
            mCloudDriveWidget->selectPendingIndexes();
        }

        if (type == NodeSelector::IncreaseOrDecrease::INCREASE &&
            actionType == MoveActionType::RESTORE)
        {
            // Check with the handle if we are in CD or Incoming
            if (ui->stackedWidget->currentWidget() == mRubbishWidget)
            {
                onbShowCloudDriveClicked();
            }
        }
    }

    if (!targetTabsInfo.incomingSharedNodes.isEmpty())
    {
        if (actionType != MoveActionType::COPY)
        {
            fillMergeFolders(targetTabsInfo.incomingSharedNodes, mIncomingSharesWidget);
        }
        else
        {
            mIncomingSharesWidget->setAsyncSelectedNodeHandle(targetTabsInfo.incomingSharedNodes);
            mIncomingSharesWidget->selectPendingIndexes();
        }

        if (type == NodeSelector::IncreaseOrDecrease::INCREASE &&
            actionType == MoveActionType::RESTORE)
        {
            // Check with the handle if we are in CD or Incoming
            if (ui->stackedWidget->currentWidget() == mRubbishWidget)
            {
                onbShowIncomingSharesClicked();
            }
        }
    }
}

void CloudDriveNodeSelector::checkMovingItems(const QList<mega::MegaHandle>& handles,
                                              int moveType,
                                              NodeSelector::IncreaseOrDecrease type)
{
    if (moveType == MoveActionType::RESTORE)
    {
        auto tabsInfo(getTabs(handles));

        if (!tabsInfo.cloudDriveNodes.isEmpty())
        {
            type == NodeSelector::IncreaseOrDecrease::INCREASE ?
                mCloudDriveWidget->increaseMovingNodes(
                    static_cast<int>(tabsInfo.cloudDriveNodes.size())) :
                mIncomingSharesWidget->decreaseMovingNodes(
                    static_cast<int>(tabsInfo.cloudDriveNodes.size()));
        }

        if (!tabsInfo.incomingSharedNodes.isEmpty())
        {
            type == NodeSelector::IncreaseOrDecrease::INCREASE ?
                mIncomingSharesWidget->increaseMovingNodes(
                    static_cast<int>(tabsInfo.incomingSharedNodes.size())) :
                mIncomingSharesWidget->decreaseMovingNodes(
                    static_cast<int>(tabsInfo.incomingSharedNodes.size()));
        }

        selectTabs(tabsInfo);

        performItemsToBeMoved(handles, type, true, false);
    }
    else if (moveType == MoveActionType::DELETE_RUBBISH)
    {
        if (type == NodeSelector::IncreaseOrDecrease::INCREASE)
        {
            mRubbishWidget->increaseMovingNodes(static_cast<int>(handles.size()));
        }
        else
        {
            mRubbishWidget->decreaseMovingNodes(static_cast<int>(handles.size()));
        }

        performItemsToBeMoved(handles, type, true, false);
    }
    else if (moveType == MoveActionType::DELETE_PERMANENTLY)
    {
        performItemsToBeMoved(handles, type, true, false);
    }
    else if (moveType == MoveActionType::COPY)
    {
        performItemsToBeMoved(handles, type, false, true);
    }
    else
    {
        performItemsToBeMoved(handles, type, true, true);
    }
}

CloudDriveNodeSelector::HandlesByTab
    CloudDriveNodeSelector::getTabs(const QList<mega::MegaHandle>& handles)
{
    HandlesByTab info;

    if (!handles.isEmpty())
    {
        for (const auto& handle: handles)
        {
            std::unique_ptr<mega::MegaNode> node(
                MegaSyncApp->getMegaApi()->getNodeByHandle(handle));
            if (node)
            {
                mega::MegaNode* checkNode(nullptr);

                std::unique_ptr<mega::MegaNode> restoreNode(
                    MegaSyncApp->getMegaApi()->getNodeByHandle(node->getRestoreHandle()));
                if (restoreNode)
                {
                    checkNode = restoreNode.get();
                }
                else
                {
                    checkNode = node.get();
                }

                if (MegaSyncApp->getMegaApi()->isInCloud(checkNode))
                {
                    info.cloudDriveNodes.append(handle);
                }
                else
                {
                    info.incomingSharedNodes.append(handle);
                }
            }
        }
    }

    return info;
}

void CloudDriveNodeSelector::selectTabs(const HandlesByTab& tabsInfo)
{
    if (!tabsInfo.cloudDriveNodes.isEmpty())
    {
        onbShowCloudDriveClicked();
        onOptionSelected(NodeSelectorTreeViewWidget::TabItem::CLOUD_DRIVE);
    }
    else if (!tabsInfo.incomingSharedNodes.isEmpty())
    {
        onbShowIncomingSharesClicked();
        onOptionSelected(NodeSelectorTreeViewWidget::TabItem::SHARES);
    }
}

////////////////////////////////
MoveBackupNodeSelector::MoveBackupNodeSelector(QWidget* parent):
    FilePickerNodeSelector(SelectTypeSPtr(new MoveBackupType), parent)
{}

void MoveBackupNodeSelector::onOkButtonClicked()
{
    accept();
}
