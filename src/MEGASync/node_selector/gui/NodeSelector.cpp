#include "NodeSelector.h"

#include "DialogOpener.h"
#include "DuplicatedNodeDialog.h"
#include "IncomingShareHeaderWidget.h"
#include "megaapi.h"
#include "MegaApplication.h"
#include "MegaNodeNames.h"
#include "MessageDialogOpener.h"
#include "NewFolderDialog.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorProxyModel.h"
#include "NodeSelectorTreeViewWidgetSpecializations.h"
#include "SearchLineEdit.h"
#include "TabSelector.h"
#include "TokenizableItems/TokenPropertySetter.h"
#include "ui_NodeSelector.h"
#include "Utilities.h"
#include "ViewLoadingScene.h"

#include <QKeyEvent>
#include <QLayout>
#include <QMessageBox>
#include <QPointer>
#include <QShortcut>

#include <optional>

using namespace mega;

NodeSelector::NodeSelector(SelectTypeSPtr selectType, QWidget* parent):
    QDialog(parent),
    mMegaApi(MegaSyncApp->getMegaApi()),
    ui(new Ui::NodeSelector),
    mSelectType(selectType),
    mDelegateListener(std::make_unique<QTMegaListener>(mMegaApi, this)),
    mInitialised(false),
    mDuplicatedType(std::nullopt),
    mDuplicatedModel(nullptr),
    mTargetWid(nullptr)
{
    ui->setupUi(this);
    ui->cbAlwaysUploadToLocation->hide();

    TabSelector::applyActionToTabSelectors(ui->wLeftPaneNS,
                                           [](TabSelector* tabSelector)
                                           {
                                               tabSelector->setIconOnly(true);
                                           });

    connect(ui->stackedWidget,
            &QStackedWidget::currentChanged,
            this,
            &NodeSelector::onCurrentWidgetChanged);

    mMegaApi->addListener(mDelegateListener.get());

    connect(ui->fIncomingShares,
            &TabSelector::clicked,
            this,
            &NodeSelector::onbShowIncomingSharesClicked,
            Qt::QueuedConnection);
    connect(ui->fCloudDrive, &TabSelector::clicked, this, &NodeSelector::onbShowCloudDriveClicked);
    connect(ui->fBackups, &TabSelector::clicked, this, &NodeSelector::onbShowBackupsFolderClicked);
    connect(ui->fRubbish, &TabSelector::clicked, this, &NodeSelector::onbShowRubbishClicked);
    connect(ui->fSearch, &TabSelector::clicked, this, &NodeSelector::onbShowSearchClicked);

    ui->fCloudDrive->connectToDropEvent(
        [this](std::shared_ptr<QDropEvent> event)
        {
            onCloudDriveTabDropped(event);
        });

    TabSelector::applyActionToTabSelectors(ui->wLeftPaneNS,
                                           [this](TabSelector* tabSelector)
                                           {
                                               if (tabSelector != ui->fBackups)
                                               {
                                                   tabSelector->setAcceptDrops(true);
                                               }
                                           });

    connect(ui->fSearch, &TabSelector::hidden, this, &NodeSelector::onfShowSearchHidden);

    ui->wSearch->hide();

    updateNodeSelectorTabs();
    onOptionSelected(CLOUD_DRIVE);

    // Left pane tokens
    {
        BaseTokens iconTokens;
        iconTokens.setNormalOff(QLatin1String("icon-secondary"));
        iconTokens.setNormalOn(QLatin1String("icon-primary"));
        auto iconTokenSetter = std::make_shared<TokenPropertySetter>(iconTokens);

        TabSelector::applyTokens(ui->wLeftPaneNS, iconTokenSetter);
    }

    if (!mSelectType->footerVisible())
    {
        ui->footer->hide();
        ui->wRightPaneNS->layout()->setContentsMargins(0, 0, 0, 14);
    }

    resetButtonsText();

    ui->bOk->setDefault(true);
    ui->bOk->setEnabled(false);

    connect(ui->bOk, &QPushButton::clicked, this, &NodeSelector::confirmSelection);
    connect(ui->bCancel, &QPushButton::clicked, this, &NodeSelector::reject);

    connect(ui->bBack,
            &QPushButton::clicked,
            this,
            [this]()
            {
                if (auto wid = getCurrentTreeViewWidget())
                {
                    wid->goBack();
                }
            });
    connect(ui->bForward,
            &QPushButton::clicked,
            this,
            [this]()
            {
                if (auto wid = getCurrentTreeViewWidget())
                {
                    wid->goForward();
                }
            });
    connect(ui->leSearchTool, &SearchLineEdit::search, this, &NodeSelector::onSearch);

    ui->incomingShareWidget->hide();

    resize(1024, 720);
    setMinimumSize(760, 400);
}

NodeSelector::~NodeSelector()
{
    // Remove duplicated node dialog if it is currently open
    if (mDuplicatedType.has_value())
    {
        DialogOpener::closeDialogsByClass<DuplicatedNodeDialog>();
    }
    mMegaApi->removeListener(mDelegateListener.get());
    delete ui;
}

void NodeSelector::init()
{
    createActionButtons();
    createSpecialisedWidgets();
    addSearch();
    onCurrentWidgetChanged(ui->stackedWidget->currentIndex());

    mInitialised = true;
}

void NodeSelector::updateNodeSelectorTabs()
{
    auto setIconOnlyTabTitle = [](TabSelector* tab, const QString& title)
    {
        tab->setTitle(QString());
        tab->setToolTip(title);
        tab->setAccessibleName(title);
    };

    setIconOnlyTabTitle(ui->fCloudDrive, MegaNodeNames::getCloudDriveName());
    setIconOnlyTabTitle(ui->fIncomingShares, MegaNodeNames::getIncomingSharesName());
    setIconOnlyTabTitle(ui->fBackups, MegaNodeNames::getBackupsName());
    setIconOnlyTabTitle(ui->fRubbish, MegaNodeNames::getRubbishName());
}

void NodeSelector::onSearch(const QString& text)
{
    ui->wSearch->show();
    ui->fSearch->setTitle(text);
    ui->fSearch->setSelected(true);

    mSearchWidget->search(text);
    onbShowSearchClicked();
}

void NodeSelector::onbNewFolderClicked()
{
    auto sourceWidget = getCurrentTreeViewWidget();

    if (!sourceWidget)
    {
        return;
    }

    auto parentNode = sourceWidget->getProxyModel()->getNode(sourceWidget->getCurrentRootIndex());
    if (!parentNode)
    {
        parentNode = MegaSyncApp->getRootNode();
        if (!parentNode)
            return;
    }

    QPointer<NewFolderDialog> dialog(new NewFolderDialog(parentNode, this));
    dialog->init();
    DialogOpener::showDialog(
        dialog,
        [this, dialog, sourceWidget]()
        {
            auto newNode = dialog->getNewNode();
            // IF the dialog return a node, there are two scenarios:
            // 1) The dialog has been accepted, a new folder has been created
            // 2) The dialog has been rejected because the folder already
            // exists. If so, select the existing folder
            if (newNode)
            {
                sourceWidget->setNewFolderInfo({newNode->getHandle(), true});

                // Focusing a widget whose top-level window is not the active
                // window makes Qt call QWindow::requestActivate()
                // (QWidgetPrivate::setFocus_sys). Wayland does not support it
                // and older Qt 5.15 builds crash there, so only force the
                // activation/focus when our window is already active. When it
                // is not (e.g. just after the NewFolderDialog closes on
                // Wayland), the compositor restores focus on its own.
                const QWindow* selectorWindow = window() ? window()->windowHandle() : nullptr;
                const bool windowAlreadyActive =
                    selectorWindow && selectorWindow == QGuiApplication::focusWindow();
                if (!Platform::getInstance()->isWayland() || windowAlreadyActive)
                {
#ifdef Q_OS_LINUX
                    // It seems that the NodeSelector is not activated when the
                    // NewFolderDialog is closed, so the ui->tMegaFolders is
                    // not correctly focused
                    qApp->setActiveWindow(this);
#endif

                    // Set the focus to the view to allow the user to press
                    // enter (or go back, in a future feature)
                    setFocus();
                }
            }
        });
}

void NodeSelector::onUiIsBlocked(bool state)
{
    ui->bCancel->setDisabled(state);
    ui->leSearchTool->setDisabled(state);
    if (state)
    {
        ui->bOk->setDisabled(true);
    }

    ui->header->setDisabled(state);
}

void NodeSelector::onSelectionChanged()
{
    auto wid = getCurrentTreeViewWidget();
    if (wid)
    {
        auto isSelectionCorrect = mSelectType->okButtonEnabled(wid, wid->getSelectedIndexes());
        ui->bOk->setEnabled(isSelectionCorrect);
    }
}

QString NodeSelector::folderNameForWidget(NodeSelectorTreeViewWidget* wid) const
{
    if (!wid)
    {
        return QString();
    }

    if (wid == mSearchWidget)
    {
        return QString();
    }

    const auto rootIndex = wid->getCurrentRootIndex();
    if (rootIndex.isValid())
    {
        return rootIndex.data(Qt::DisplayRole).toString();
    }

    switch (ui->stackedWidget->indexOf(wid))
    {
        case CLOUD_DRIVE:
            return MegaNodeNames::getCloudDriveName();
        case SHARES:
            return MegaNodeNames::getIncomingSharesName();
        case BACKUPS:
            return MegaNodeNames::getBackupsName();
        case RUBBISH:
            return MegaNodeNames::getRubbishName();
        case SEARCH:
            return QString();
        default:
            return QString();
    }
}

void NodeSelector::applyNavigationButtonsState(NodeSelectorTreeViewWidget* wid)
{
    if (!wid)
    {
        return;
    }

    ui->navigationButtons->setVisible(wid->shouldShowNavigationButtons());
    ui->bBack->setEnabled(wid->canGoBack());
    ui->bForward->setEnabled(wid->canGoForward());
}

void NodeSelector::applyHeaderFolderInfoState(NodeSelectorTreeViewWidget* wid)
{
    if (!wid)
    {
        return;
    }

    const auto incomingInfo = wid->incomingShareHeaderData();
    const auto showIncomingInfo = incomingInfo.has_value();
    ui->lFolderName->setText(folderNameForWidget(wid));
    ui->incomingShareWidget->setVisible(showIncomingInfo);
    ui->lFolderName->setVisible(!showIncomingInfo);

    if (incomingInfo)
    {
        ui->incomingShareWidget->setData(*incomingInfo);
    }
    else
    {
        ui->incomingShareWidget->clear();
    }
}

void NodeSelector::applyHeaderButtonsState(NodeSelectorTreeViewWidget* wid)
{
    if (!wid)
    {
        return;
    }

    mSelectType->checkActionButtonsVisibility(wid);
}

void NodeSelector::refreshHeaderButtons(NodeSelectorTreeViewWidget* wid)
{
    if (!wid || wid != getCurrentTreeViewWidget())
    {
        return;
    }

    applyHeaderButtonsState(wid);
    applyNavigationButtonsState(wid);
}

void NodeSelector::refreshHeader(NodeSelectorTreeViewWidget* wid)
{
    if (!wid || wid != getCurrentTreeViewWidget())
    {
        return;
    }

    applyHeaderButtonsState(wid);
    applyNavigationButtonsState(wid);
    applyHeaderFolderInfoState(wid);
}

void NodeSelector::showDefaultUploadOption(bool show)
{
    ui->cbAlwaysUploadToLocation->setVisible(show);
}

void NodeSelector::setDefaultUploadOption(bool value)
{
    ui->cbAlwaysUploadToLocation->setChecked(value);
}

bool NodeSelector::getDefaultUploadOption()
{
    return ui->cbAlwaysUploadToLocation->isChecked();
}

bool NodeSelector::event(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        resetButtonsText();
        updateNodeSelectorTabs();
        onLanguageChangeEvent();
        if (auto widg = getCurrentTreeViewWidget())
        {
            if (mSelectType)
            {
                mSelectType->updateCustomButtonsText(widg);
            }
            refreshHeader(widg);
        }
    }

    return QDialog::event(event);
}

void NodeSelector::mousePressEvent(QMouseEvent* event)
{
    for (int page = 0; page < ui->stackedWidget->count(); ++page)
    {
        auto viewContainer = getTreeViewWidget(page);
        if (viewContainer)
        {
            viewContainer->clearSelection();
        }
    }

    QDialog::mousePressEvent(event);
}

void NodeSelector::confirmSelection()
{
    auto currentViewContainer = getCurrentTreeViewWidget();
    for (int page = 0; page < ui->stackedWidget->count(); ++page)
    {
        auto viewContainer = getTreeViewWidget(page);
        if (viewContainer && viewContainer != currentViewContainer)
        {
            viewContainer->abort();
        }
    }

    onOkButtonClicked();
}

void NodeSelector::onfShowSearchHidden()
{
    ui->wSearch->hide();
    ui->fSearch->setTitle(QString());
    ui->leSearchTool->onClearClicked();
    mSearchWidget->stopSearch();

    if (getCurrentTreeViewWidget() == mSearchWidget)
    {
        onbShowCloudDriveClicked();
    }
}

void NodeSelector::onItemsAboutToBeMoved(const QList<mega::MegaHandle>& handles, int)
{
    performItemsToBeMoved(handles, IncreaseOrDecrease::INCREASE, true, true);
}

void NodeSelector::onItemsAboutToBeMovedFailed(const QList<mega::MegaHandle>& handles, int)
{
    performItemsToBeMoved(handles, IncreaseOrDecrease::DECREASE, true, true);
}

void NodeSelector::performItemsToBeMoved(const QList<mega::MegaHandle>& handles,
                                         IncreaseOrDecrease type,
                                         bool blockSource,
                                         bool blockTarget)
{
    if (handles.isEmpty())
    {
        return;
    }

    mTargetWid = nullptr;
    mSourceWids.clear();

    // IF we want to block the source or target, set values to false in order to look for them
    bool foundSource(!blockSource);
    bool foundTarget(!blockTarget);

    auto senderModel(dynamic_cast<NodeSelectorModel*>(sender()));

    auto targetOrSourceFound = [type](NodeSelectorTreeViewWidget* wid, int nodesUpdateToReceive)
    {
        if (wid)
        {
            if (type == IncreaseOrDecrease::INCREASE)
            {
                wid->increaseMovingNodes(nodesUpdateToReceive);
            }
            else
            {
                wid->decreaseMovingNodes(nodesUpdateToReceive);
            }
        }
    };

    auto findSource = [this, handles](NodeSelectorTreeViewWidget* wid)
    {
        if (wid->areItemsAboutToBeMovedFromHere(handles.first()))
        {
            mSourceWids.append(wid);
        }
    };

    for (int index = 0; index < ui->stackedWidget->count(); ++index)
    {
        if (auto wid = dynamic_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->widget(index)))
        {
            if (!foundTarget && wid->getProxyModel()->getMegaModel() == senderModel)
            {
                foundTarget = true;
                mTargetWid = wid;

                findSource(wid);
            }
            else if (!foundSource)
            {
                findSource(wid);
            }
        }
    }

    targetOrSourceFound(mTargetWid, static_cast<int>(handles.size()));

    for (auto& wid: mSourceWids)
    {
        if (wid != mTargetWid)
        {
            targetOrSourceFound(wid, static_cast<int>(handles.size()));
        }
    }
}

void NodeSelector::onOptionSelected(int index)
{
    switch (index)
    {
        case NodeSelector::CLOUD_DRIVE:
            ui->fCloudDrive->setSelected(true);
            break;
        case NodeSelector::SHARES:
            ui->fIncomingShares->setSelected(true);
            break;
        case NodeSelector::BACKUPS:
            ui->fBackups->setSelected(true);
            break;
        case NodeSelector::RUBBISH:
            ui->fRubbish->setSelected(true);
            break;
        case NodeSelector::SEARCH:
            ui->fSearch->setSelected(true);
            break;
        default:
            break;
    }
}

void NodeSelector::onCloudDriveTabDropped(std::shared_ptr<QDropEvent> event)
{
    getTreeViewWidget(CLOUD_DRIVE)->dropIntoRootIndex(event.get());
}

void NodeSelector::onbShowCloudDriveClicked()
{
    ui->stackedWidget->setCurrentIndex(CLOUD_DRIVE);
}

void NodeSelector::onbShowRubbishClicked()
{
    ui->stackedWidget->setCurrentIndex(RUBBISH);
}

void NodeSelector::onbShowIncomingSharesClicked()
{
    if (ui->fIncomingShares->isVisible())
    {
        ui->stackedWidget->setCurrentIndex(SHARES);
    }
}

void NodeSelector::onbShowBackupsFolderClicked()
{
    if (ui->fBackups->isVisible())
    {
        ui->stackedWidget->setCurrentIndex(BACKUPS);
    }
}

void NodeSelector::onbShowSearchClicked()
{
    if (ui->fSearch->isVisible())
    {
        ui->stackedWidget->setCurrentWidget(mSearchWidget);
    }
}

void NodeSelector::shortCutConnects(int ignoreThis)
{
    // Provide quick access shortcuts for the two panes via Ctrl+1,2
    // Ctrl is auto-magically translated to CMD key by Qt on macOS
    for (int i = 0; i <= BACKUPS; ++i)
    {
        if (i != ignoreThis)
        {
            QShortcut* shortcut =
                new QShortcut(QKeySequence(QString::fromLatin1("Ctrl+%1").arg(i + 1)), this);
            QObject::connect(shortcut,
                             &QShortcut::activated,
                             this,
                             [=]()
                             {
                                 onOptionSelected(i);
                             });
        }
    }
}

void NodeSelector::resetButtonsText()
{
    // Done here to re-use contexts
    ui->bOk->setText(QCoreApplication::translate("NodeSelectorTreeViewWidget", "Ok"));
    ui->bCancel->setText(QCoreApplication::translate("NodeSelectorTreeViewWidget", "Cancel"));
}

NodeSelectorTreeViewWidget* NodeSelector::getTreeViewWidget(int page) const
{
    return dynamic_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->widget(page));
}

NodeSelectorTreeViewWidget* NodeSelector::getTreeViewWidget(QObject* object) const
{
    return dynamic_cast<NodeSelectorTreeViewWidget*>(object);
}

NodeSelectorTreeViewWidget* NodeSelector::getCurrentTreeViewWidget() const
{
    return getTreeViewWidget(ui->stackedWidget->currentWidget());
}

MegaHandle NodeSelector::getSelectedNodeHandle()
{
    auto tree_view = getCurrentTreeViewWidget();
    return tree_view ? tree_view->getSelectedNodeHandle() : mega::INVALID_HANDLE;
}

QList<MegaHandle> NodeSelector::getMultiSelectionNodeHandle()
{
    auto tree_view = getCurrentTreeViewWidget();
    return tree_view ? tree_view->getMultiSelectionNodeHandle() : QList<MegaHandle>();
}

void NodeSelector::closeEvent(QCloseEvent* event)
{
    for (int page = 0; page < ui->stackedWidget->count(); ++page)
    {
        auto viewContainer = getTreeViewWidget(page);
        if (viewContainer)
        {
            viewContainer->abort();
            if (viewContainer->getProxyModel()->isModelProcessing())
            {
                connect(viewContainer->getProxyModel()->getMegaModel(),
                        &NodeSelectorModel::blockUi,
                        this,
                        [this](bool blocked)
                        {
                            if (!blocked)
                            {
                                close();
                            }
                        });
                event->ignore();
                return;
            }
        }
    }

    QDialog::closeEvent(event);
}

std::shared_ptr<MegaNode> NodeSelector::getSelectedNode()
{
    auto node = std::shared_ptr<MegaNode>(mMegaApi->getNodeByHandle(getSelectedNodeHandle()));
    return node;
}

void NodeSelector::showNotFoundNodeMessageBox()
{
    MessageDialogInfo msgInfo;
    msgInfo.descriptionText =
        tr("The item you selected has been removed. To reselect, close this window and try again.");
    MessageDialogOpener::warning(msgInfo);
}

void NodeSelector::createActionButtons()
{
    // Create buttons
    auto buttons = mSelectType->addActionButtons();

    for (auto it = buttons.cbegin(); it != buttons.cend(); ++it)
    {
        if (it.value())
        {
            connect(it.value(),
                    &QPushButton::clicked,
                    this,
                    [this, id = it.key()]()
                    {
                        onCustomButtonClicked(id);
                    });
        }

        ui->customButtonsLayout->addWidget(it.value());
    }
}

void NodeSelector::initSpecialisedWidgets(NodeSelectorTreeViewWidget* viewContainer)
{
    if (viewContainer)
    {
        connect(viewContainer,
                &NodeSelectorTreeViewWidget::onCustomButtonClicked,
                this,
                &NodeSelector::onCustomButtonClicked,
                Qt::UniqueConnection);
        connect(viewContainer,
                &NodeSelectorTreeViewWidget::uiIsBlocked,
                this,
                &NodeSelector::onUiIsBlocked,
                Qt::UniqueConnection);
        connect(viewContainer,
                &NodeSelectorTreeViewWidget::enterKeyPressed,
                ui->bOk,
                &QPushButton::click,
                Qt::UniqueConnection);
        connect(viewContainer,
                &NodeSelectorTreeViewWidget::newFolderRequested,
                this,
                &NodeSelector::onbNewFolderClicked,
                Qt::UniqueConnection);
        connect(viewContainer,
                &NodeSelectorTreeViewWidget::viewReady,
                this,
                &NodeSelector::performNodeSelection);

        auto model = viewContainer->getProxyModel()->getMegaModel();

        connect(model,
                &NodeSelectorModel::itemsAboutToBeMoved,
                this,
                &NodeSelector::onItemsAboutToBeMoved);

        connect(model,
                &NodeSelectorModel::itemsAboutToBeMovedFailed,
                this,
                &NodeSelector::onItemsAboutToBeMovedFailed);

        connect(model,
                &NodeSelectorModel::itemRequestsFinished,
                this,
                &NodeSelector::onItemRequestsFinished);

        connect(model,
                &NodeSelectorModel::itemsAboutToBeRestored,
                this,
                &NodeSelector::onItemsAboutToBeRestored);

        connect(model,
                &NodeSelectorModel::itemAboutToBeReplaced,
                this,
                &NodeSelector::onItemAboutToBeReplaced);

        connect(model,
                &NodeSelectorModel::itemsAboutToBeMerged,
                this,
                &NodeSelector::onItemsAboutToBeMerged);

        connect(model,
                &NodeSelectorModel::itemMergeFinished,
                this,
                &NodeSelector::onItemMergeFinished);

        connect(model,
                &NodeSelectorModel::itemsAboutToBeMergedFailed,
                this,
                &NodeSelector::onItemsAboutToBeMergedFailed);

        connect(model,
                &NodeSelectorModel::showMessageBox,
                this,
                [this](MessageDialogInfo info)
                {
                    info.parent = this;
                    MessageDialogOpener::warning(info);
                });

        connect(model,
                &NodeSelectorModel::showDuplicatedNodeDialog,
                this,
                [this, model](std::shared_ptr<ConflictTypes> conflicts, MoveActionType type)
                {
                    mDuplicatedModel = model;
                    mDuplicatedConflicts = conflicts;
                    mDuplicatedType = type;

                    auto checkUploadNameDialog = new DuplicatedNodeDialog(this);
                    checkUploadNameDialog->setConflicts(conflicts);

                    DialogOpener::showDialog<DuplicatedNodeDialog, NodeSelector>(
                        checkUploadNameDialog,
                        this,
                        &NodeSelector::onShowDuplicatedNodeDialog);
                });

        mSelectType->initTreeViewWidget(viewContainer);
    }
}

bool NodeSelector::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::DragEnter)
    {
        if (auto button = dynamic_cast<QPushButton*>(obj))
        {
            button->click();
        }
    }

    return QDialog::eventFilter(obj, event);
}

void NodeSelector::setSelectedNodeHandle(std::shared_ptr<MegaNode> node)
{
    mNodeToBeSelected = node;
}

void NodeSelector::performNodeSelection()
{
    if (mNodeToBeSelected)
    {
        std::optional<TabItem> option = selectedNodeTab();

        if (option.has_value())
        {
            auto optionValue(option.value());
            if (ui->stackedWidget->currentIndex() == optionValue)
            {
                onCurrentWidgetChanged(optionValue);
            }
            else
            {
                onOptionSelected(optionValue);
            }
        }

        // Disconnect all connections as soon as the node was selected
        if (!mNodeToBeSelected)
        {
            for (int index = 0; index < ui->stackedWidget->count(); ++index)
            {
                if (auto wid =
                        dynamic_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->widget(index)))
                {
                    disconnect(wid,
                               &NodeSelectorTreeViewWidget::viewReady,
                               this,
                               &NodeSelector::performNodeSelection);
                }
            }
        }
    }
}

std::optional<NodeSelector::TabItem> NodeSelector::selectedNodeTab()
{
    if (mNodeToBeSelected)
    {
        std::optional<TabItem> option;

        if (mMegaApi->isInCloud(mNodeToBeSelected.get()))
        {
            option = CLOUD_DRIVE;
        }
        else if (mMegaApi->isInVault(mNodeToBeSelected.get()))
        {
            option = BACKUPS;
        }
        else if (mMegaApi->isInRubbish(mNodeToBeSelected.get()))
        {
            option = RUBBISH;
        }
        else
        {
            option = SHARES;
        }

        if (option.has_value())
        {
            return option.value();
        }
    }

    return std::nullopt;
}

void NodeSelector::onCurrentWidgetChanged(int index)
{
    if (auto wid = dynamic_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->widget(index)))
    {
        onOptionSelected(index);

        disconnect(mViewStateConnection);
        disconnect(mViewButtonsStateConnection);

        if (mNodeToBeSelected)
        {
            wid->clearSelection();
            wid->setSelectedNodeHandle(mNodeToBeSelected->getHandle());
            mNodeToBeSelected.reset();
        }

        wid->treeViewWidgetSelected();

        disconnect(mSelectionChangedConnection);
        mSelectionChangedConnection = connect(wid,
                                              &NodeSelectorTreeViewWidget::selectionHasChanged,
                                              this,
                                              &NodeSelector::onSelectionChanged,
                                              Qt::UniqueConnection);
        mViewStateConnection = connect(wid,
                                       &NodeSelectorTreeViewWidget::viewStateChanged,
                                       this,
                                       [this, wid]()
                                       {
                                           refreshHeader(wid);
                                       });
        mViewButtonsStateConnection = connect(wid,
                                              &NodeSelectorTreeViewWidget::viewButtonsStateChanged,
                                              this,
                                              [this, wid]()
                                              {
                                                  refreshHeaderButtons(wid);
                                              });

        refreshHeader(wid);
        onSelectionChanged();
    }
}

void NodeSelector::onShowDuplicatedNodeDialog(QPointer<DuplicatedNodeDialog>)
{
    if (mDuplicatedType.has_value())
    {
        mDuplicatedModel->processNodesAfterConflictCheck(
            mDuplicatedConflicts,
            static_cast<MoveActionType>(mDuplicatedType.value()));
        mDuplicatedType = std::nullopt;
        mDuplicatedModel = nullptr;
        mDuplicatedConflicts.reset();
    }
}

void NodeSelector::onNodesUpdate(mega::MegaApi* api, mega::MegaNodeList* nodes)
{
    for (int index = 0; index < ui->stackedWidget->count(); ++index)
    {
        if (auto wid = dynamic_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->widget(index)))
        {
            if (wid != mSearchWidget || !ui->fSearch->isHidden())
            {
                wid->onNodesUpdate(api, nodes);
            }
        }
    }
}

void NodeSelector::createSpecialisedWidgets()
{
    const QList<QPair<TabType, TabSelector*>> tabOrder = {
        {TabType::CLOUD_DRIVE, ui->fCloudDrive},
        {TabType::INCOMING_SHARE, ui->fIncomingShares},
        {TabType::BACKUP, ui->fBackups},
        {TabType::RUBBISH, ui->fRubbish},
    };

    auto allowed = mSelectType->allowedTabTypes();

    for (const auto& [type, button]: tabOrder)
    {
        if (allowed & type)
        {
            button->show();
            addWidgetForTabType(type);
        }
        else
        {
            button->hide();
        }
    }

    afterWidgetsCreated();
}

void NodeSelector::addWidgetForTabType(TabType type)
{
    switch (type)
    {
        case TabType::CLOUD_DRIVE:
        {
            addCloudDrive();
            break;
        }
        case TabType::INCOMING_SHARE:
        {
            addIncomingShares();
            break;
        }
        case TabType::BACKUP:
        {
            addBackups();
            break;
        }
        case TabType::RUBBISH:
        {
            addRubbish();
            break;
        }
        default:
        {
            break;
        }
    }
}

void NodeSelector::addCloudDrive()
{
    mCloudDriveWidget = new NodeSelectorTreeViewWidgetCloudDrive(mSelectType);
    mCloudDriveWidget->init();
    initSpecialisedWidgets(mCloudDriveWidget);
    mCloudDriveWidget->setObjectName(QString::fromUtf8("CloudDrive"));
    ui->stackedWidget->addWidget(mCloudDriveWidget);
}

void NodeSelector::addIncomingShares()
{
    mIncomingSharesWidget = new NodeSelectorTreeViewWidgetIncomingShares(mSelectType);
    mIncomingSharesWidget->init();
    initSpecialisedWidgets(mIncomingSharesWidget);
    mIncomingSharesWidget->setObjectName(QString::fromUtf8("IncomingShares"));
    ui->stackedWidget->addWidget(mIncomingSharesWidget);
}

void NodeSelector::addBackups()
{
    mBackupsWidget = new NodeSelectorTreeViewWidgetBackups(mSelectType);
    mBackupsWidget->init();
    initSpecialisedWidgets(mBackupsWidget);
    mBackupsWidget->setObjectName(QString::fromUtf8("Backups"));
    ui->stackedWidget->addWidget(mBackupsWidget);
}

void NodeSelector::addSearch()
{
    mSearchWidget = new NodeSelectorTreeViewWidgetSearch(mSelectType);
    mSearchWidget->init();
    initSpecialisedWidgets(mSearchWidget);
    mSearchWidget->setObjectName(QString::fromUtf8("Search"));
    connect(mSearchWidget,
            &NodeSelectorTreeViewWidgetSearch::nodeDoubleClicked,
            this,
            [this](std::shared_ptr<MegaNode> node)
            {
                setSelectedNodeHandle(node);
                performNodeSelection();
            });
    ui->stackedWidget->addWidget(mSearchWidget);
}

void NodeSelector::addRubbish()
{
    mRubbishWidget = new NodeSelectorTreeViewWidgetRubbish(mSelectType);
    mRubbishWidget->init();
    initSpecialisedWidgets(mRubbishWidget);
    mRubbishWidget->setObjectName(QString::fromUtf8("Rubbish"));
    ui->stackedWidget->addWidget(mRubbishWidget);
}

void NodeSelector::onCustomButtonClicked(uint id)
{
    if (id == CloudDriveType::NewFolder)
    {
        onbNewFolderClicked();
    }
}
