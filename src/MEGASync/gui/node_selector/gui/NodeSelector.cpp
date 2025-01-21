#include "NodeSelector.h"

#include "DialogOpener.h"
#include "DuplicatedNodeDialog.h"
#include "megaapi.h"
#include "MegaApplication.h"
#include "MegaNodeNames.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorProxyModel.h"
#include "NodeSelectorTreeViewWidgetSpecializations.h"
#include "QMegaMessageBox.h"
#include "ui_NodeSelector.h"
#include "Utilities.h"
#include "ViewLoadingScene.h"

#include <QKeyEvent>
#include <QMessageBox>
#include <QPointer>
#include <QShortcut>

#include <optional>

using namespace mega;

const char* ITS_ON_NS = "itsOn";

NodeSelector::NodeSelector(SelectTypeSPtr selectType, QWidget* parent):
    QDialog(parent),
    mMegaApi(MegaSyncApp->getMegaApi()),
    ui(new Ui::NodeSelector),
    mSelectType(selectType),
    mDelegateListener(std::make_unique<QTMegaListener>(mMegaApi, this)),
    mInitialised(false),
    mNodeToBeSelected(mega::INVALID_HANDLE)
{
    ui->setupUi(this);

    mMegaApi->addListener(mDelegateListener.get());

    connect(ui->bShowIncomingShares, &QPushButton::clicked, this, &NodeSelector::onbShowIncomingSharesClicked, Qt::QueuedConnection);
    connect(ui->bShowCloudDrive, &QPushButton::clicked, this, &NodeSelector::onbShowCloudDriveClicked);
    connect(ui->bShowBackups, &QPushButton::clicked, this, &NodeSelector::onbShowBackupsFolderClicked);
    connect(ui->bSearchNS, &QPushButton::clicked, this, &NodeSelector::onbShowSearchClicked);
    connect(ui->bRubbish, &QPushButton::clicked, this, &NodeSelector::onbShowRubbishClicked);

    foreach(auto& button, ui->wLeftPaneNS->findChildren<QAbstractButton*>())
    {
        button->installEventFilter(this);
        mButtonIconManager.addButton(button);
    }

    QColor shadowColor (188, 188, 188);
    mShadowTab = new QGraphicsDropShadowEffect(ui->buttonGroup);
    mShadowTab->setBlurRadius(10.);
    mShadowTab->setXOffset(0.);
    mShadowTab->setYOffset(0.);
    mShadowTab->setColor(shadowColor);
    mShadowTab->setEnabled(true);

    mTabFramesToggleGroup[SEARCH] = ui->fSearchStringNS;
    mTabFramesToggleGroup[BACKUPS] = ui->fBackups;
    mTabFramesToggleGroup[SHARES] = ui->fIncomingShares;
    mTabFramesToggleGroup[CLOUD_DRIVE] = ui->fCloudDrive;
    mTabFramesToggleGroup[RUBBISH] = ui->fRubbish;
    ui->wSearchNS->hide();
    ui->bSearchNS->hide();
    ui->fRubbish->hide();
    setAllFramesItsOnProperty();

    updateNodeSelectorTabs();
    onOptionSelected(CLOUD_DRIVE);
}

NodeSelector::~NodeSelector()
{
    mMegaApi->removeListener(mDelegateListener.get());
    delete ui;
}

void NodeSelector::setAllFramesItsOnProperty()
{
    for (auto tabFrame : qAsConst(mTabFramesToggleGroup))
    {
        tabFrame->setProperty(ITS_ON_NS, false);
    }
}

void NodeSelector::updateNodeSelectorTabs()
{
    ui->bShowCloudDrive->setText(MegaNodeNames::getCloudDriveName());
    ui->bShowIncomingShares->setText(MegaNodeNames::getIncomingSharesName());
    ui->bShowBackups->setText(MegaNodeNames::getBackupsName());
    ui->bRubbish->setText(MegaNodeNames::getRubbishName());
}

void NodeSelector::onSearch(const QString &text)
{
    ui->bSearchNS->setText(text);
    ui->wSearchNS->setVisible(true);
    ui->bSearchNS->setVisible(true);

    mSearchWidget->search(text);
    mSearchWidget->setSearchText(text);
    onbShowSearchClicked();
    ui->bSearchNS->setChecked(true);

    auto senderViewWidget = getTreeViewWidget(sender());
    if (senderViewWidget && senderViewWidget != mSearchWidget)
    {
        senderViewWidget->clearSearchText();
    }
}

void NodeSelector::showDefaultUploadOption(bool show)
{
    for(int page = 0; page < ui->stackedWidget->count(); ++page)
    {
        auto viewContainer = getTreeViewWidget(page);
        if(viewContainer)
        {
            viewContainer->showDefaultUploadOption(show);
        }
    }
}

void NodeSelector::setDefaultUploadOption(bool value)
{
    for(int page = 0; page < ui->stackedWidget->count(); ++page)
    {
        auto viewContainer = getTreeViewWidget(page);
        if(viewContainer)
        {
            viewContainer->setDefaultUploadOption(value);
        }
    }
}

bool NodeSelector::getDefaultUploadOption()
{
    return mCloudDriveWidget->getDefaultUploadOption();
}

void NodeSelector::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        updateNodeSelectorTabs();
    }
    QDialog::changeEvent(event);
}

void NodeSelector::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    {
        e->ignore();
    }
    }
}

void NodeSelector::mousePressEvent(QMouseEvent *event)
{
    for(int page = 0; page < ui->stackedWidget->count(); ++page)
    {
        auto viewContainer = getTreeViewWidget(page);
        if (viewContainer)
        {
            viewContainer->clearSelection();
        }
    }

    QDialog::mousePressEvent(event);
}

void NodeSelector::showEvent(QShowEvent*)
{
    if(!mInitialised)
    {
        createSpecialisedWidgets();
        addSearch();
        initSpecialisedWidgets();

        mInitialised = true;
    }
}

void NodeSelector::onbOkClicked()
{
    for(int page = 0; page < ui->stackedWidget->count(); ++page)
    {
        auto viewContainer = getTreeViewWidget(page);
        if (viewContainer && viewContainer != getCurrentTreeViewWidget())
        {
            viewContainer->abort();
        }
    }

    checkSelection();
}

void NodeSelector::on_tClearSearchResultNS_clicked()
{
    ui->wSearchNS->hide();
    ui->bSearchNS->hide();
    ui->bSearchNS->setText(QString());
    mSearchWidget->stopSearch();
    if (getCurrentTreeViewWidget() == mSearchWidget)
    {
        onbShowCloudDriveClicked();
    }
}

void NodeSelector::onUpdateLoadingMessage(std::shared_ptr<MessageInfo> message)
{
    auto viewContainer = getCurrentTreeViewWidget();
    if (viewContainer && viewContainer->getProxyModel()->getMegaModel() == sender())
    {
        viewContainer->updateLoadingMessage(message);
    }
}

void NodeSelector::onItemsRestoreRequested(const QList<mega::MegaHandle>& handles)
{
    auto rubbishViewContainer =
        dynamic_cast<NodeSelectorTreeViewWidgetRubbish*>(ui->stackedWidget->widget(RUBBISH));
    if (rubbishViewContainer)
    {
        rubbishViewContainer->restoreItems(handles);
    }
}

void NodeSelector::onItemsRestored(const QList<mega::MegaHandle>& handles)
{
    auto cloudDriveViewContainer = dynamic_cast<NodeSelectorTreeViewWidgetCloudDrive*>(ui->stackedWidget->widget(CLOUD_DRIVE));
    if(cloudDriveViewContainer)
    {
        cloudDriveViewContainer->itemsRestored(handles);
        onbShowCloudDriveClicked();
    }
}

void NodeSelector::onOptionSelected(int index)
{
    switch (index)
    {
        case NodeSelector::CLOUD_DRIVE:
            ui->bShowCloudDrive->click();
            break;
        case NodeSelector::SHARES:
            ui->bShowIncomingShares->click();
            break;
        case NodeSelector::BACKUPS:
            ui->bShowBackups->click();
            break;
        default:
            break;
    }
}

void NodeSelector::onbShowCloudDriveClicked()
{
    ui->stackedWidget->setCurrentIndex(CLOUD_DRIVE);
    setToggledStyle(CLOUD_DRIVE);
}

void NodeSelector::onbShowRubbishClicked()
{
    ui->stackedWidget->setCurrentIndex(RUBBISH);
    setToggledStyle(RUBBISH);
}

void NodeSelector::onbShowIncomingSharesClicked()
{
    if(ui->bShowIncomingShares->isVisible())
    {
        ui->stackedWidget->setCurrentIndex(SHARES);
        setToggledStyle(SHARES);
    }
}

void NodeSelector::onbShowBackupsFolderClicked()
{
    if(ui->bShowBackups->isVisible())
    {
        ui->stackedWidget->setCurrentIndex(BACKUPS);
        setToggledStyle(BACKUPS);
    }
}

void NodeSelector::onbShowSearchClicked()
{
    if(ui->bSearchNS->isVisible())
    {
        ui->stackedWidget->setCurrentWidget(mSearchWidget);
        setToggledStyle(SEARCH);
    }
}

void NodeSelector::shortCutConnects(int ignoreThis)
{
    // Provide quick access shortcuts for the two panes via Ctrl+1,2
    // Ctrl is auto-magically translated to CMD key by Qt on macOS
    for (int i = 0; i <= BACKUPS; ++i)
    {
        if(i != ignoreThis)
        {
            QShortcut *shortcut = new QShortcut(QKeySequence(QString::fromLatin1("Ctrl+%1").arg(i+1)), this);
            QObject::connect(shortcut, &QShortcut::activated, this, [=](){ onOptionSelected(i); });
        }
    }
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
    for(int page = 0; page < ui->stackedWidget->count(); ++page)
    {
        auto viewContainer = getTreeViewWidget(page);
        if(viewContainer)
        {
            viewContainer->abort();
            if(viewContainer->getProxyModel()->isModelProcessing())
            {
                connect(viewContainer->getProxyModel()->getMegaModel(), &NodeSelectorModel::blockUi, this, [this](bool blocked){
                    if(!blocked)
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

void NodeSelector::setToggledStyle(TabItem item)
{
    setAllFramesItsOnProperty();

    auto btn = mTabFramesToggleGroup[item]->findChildren<QPushButton*>();
    if(btn.size() > 0)
    {
        btn.at(0)->setChecked(true);
    }

    mTabFramesToggleGroup[item]->setProperty(ITS_ON_NS, true);
    mTabFramesToggleGroup[item]->setGraphicsEffect(mShadowTab);

    // Reload QSS because it is glitchy
    ui->wLeftPaneNS->setStyleSheet(ui->wLeftPaneNS->styleSheet());
}

std::shared_ptr<MegaNode> NodeSelector::getSelectedNode()
{
    auto node = std::shared_ptr<MegaNode>(mMegaApi->getNodeByHandle(getSelectedNodeHandle()));
    return node;
}

void NodeSelector::showNotFoundNodeMessageBox()
{
    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.title = QMegaMessageBox::errorTitle();
    msgInfo.text = tr("The item you selected has been removed. To reselect, close this window and try again.");
    QMegaMessageBox::warning(msgInfo);
}

void NodeSelector::initSpecialisedWidgets()
{
    NodeSelectorModel* model(nullptr);

    connect(mSearchWidget,
            &NodeSelectorTreeViewWidgetSearch::nodeDoubleClicked,
            this,
            &NodeSelector::setSelectedNodeHandle);

    for(int page = 0; page < ui->stackedWidget->count(); ++page)
    {
        auto viewContainer = getTreeViewWidget(page);
        if(viewContainer)
        {
            viewContainer->init();
            connect(viewContainer, &NodeSelectorTreeViewWidget::onCustomBottomButtonClicked, this, &NodeSelector::onCustomBottomButtonClicked, Qt::UniqueConnection);
            connect(viewContainer, &NodeSelectorTreeViewWidget::okBtnClicked, this, &NodeSelector::onbOkClicked, Qt::UniqueConnection);
            connect(viewContainer, &NodeSelectorTreeViewWidget::cancelBtnClicked, this, &NodeSelector::reject, Qt::UniqueConnection);
            connect(viewContainer, &NodeSelectorTreeViewWidget::onSearch, this, &NodeSelector::onSearch, Qt::UniqueConnection);
            if(auto rubbishWidget = qobject_cast<NodeSelectorTreeViewWidgetRubbish*>(viewContainer))
            {
                connect(rubbishWidget, &NodeSelectorTreeViewWidgetRubbish::itemsRestoreRequested, this, &NodeSelector::onItemsRestoreRequested, Qt::UniqueConnection);
                connect(rubbishWidget, &NodeSelectorTreeViewWidgetRubbish::itemsRestored, this, &NodeSelector::onItemsRestored, Qt::UniqueConnection);
            }

            model = viewContainer->getProxyModel()->getMegaModel();

            connect(model,
                    &NodeSelectorModel::updateLoadingMessage,
                    this,
                    &NodeSelector::onUpdateLoadingMessage);

            connect(model,
                    &NodeSelectorModel::showMessageBox,
                    this,
                    [this](QMegaMessageBox::MessageBoxInfo info)
                    {
                        info.parent = this;
                        QMegaMessageBox::warning(info);
                    });
            connect(model,
                    &NodeSelectorModel::showDuplicatedNodeDialog,
                    this,
                    [this, model](std::shared_ptr<ConflictTypes> conflicts)
                    {
                        auto checkUploadNameDialog = new DuplicatedNodeDialog(this);
                        checkUploadNameDialog->setConflicts(conflicts);

                        DialogOpener::showDialog<DuplicatedNodeDialog>(
                            checkUploadNameDialog,
                            [model, conflicts]()
                            {
                                model->moveNodesAfterConflictCheck(conflicts);
                            });
                    });
        }
    }

    auto treeViewWidget = getCurrentTreeViewWidget();
    if (treeViewWidget && mNodeToBeSelected != mega::INVALID_HANDLE)
    {
        treeViewWidget->setSelectedNodeHandle(mNodeToBeSelected);
        mNodeToBeSelected = mega::INVALID_HANDLE;
    }
}

bool NodeSelector::eventFilter(QObject* obj, QEvent* event)
{
    if(event->type() == QEvent::DragEnter)
    {
        if(auto button = dynamic_cast<QPushButton*>(obj))
        {
            button->click();
        }
    }

    return QDialog::eventFilter(obj,event);
}

void NodeSelector::setSelectedNodeHandle(std::shared_ptr<MegaNode> node)
{
    if (node)
    {
        std::optional<TabItem> option;

        if (mMegaApi->isInCloud(node.get()))
        {
            option = CLOUD_DRIVE;
        }
        else if (mMegaApi->isInVault(node.get()))
        {
            option = BACKUPS;
        }
        else if (mMegaApi->isInShare(node.get()))
        {
            option = SHARES;
        }

        if (option.has_value())
        {
            onOptionSelected(option.value());
            mNodeToBeSelected = node->getHandle();
        }
    }
}

void NodeSelector::onNodesUpdate(mega::MegaApi* api, mega::MegaNodeList* nodes)
{
    mCloudDriveWidget->onNodesUpdate(api, nodes);
    mIncomingSharesWidget->onNodesUpdate(api, nodes);
    mBackupsWidget->onNodesUpdate(api, nodes);
    mSearchWidget->onNodesUpdate(api, nodes);
    mRubbishWidget->onNodesUpdate(api, nodes);
}

void NodeSelector::addCloudDrive()
{
    mCloudDriveWidget = new NodeSelectorTreeViewWidgetCloudDrive(mSelectType);
    mCloudDriveWidget->setObjectName(QString::fromUtf8("CloudDrive"));
    ui->stackedWidget->addWidget(mCloudDriveWidget);
}

void NodeSelector::addIncomingShares()
{
    mIncomingSharesWidget = new NodeSelectorTreeViewWidgetIncomingShares(mSelectType);
    mIncomingSharesWidget->setObjectName(QString::fromUtf8("IncomingShares"));
    ui->stackedWidget->addWidget(mIncomingSharesWidget);
}

void NodeSelector::addBackups()
{
    mBackupsWidget = new NodeSelectorTreeViewWidgetBackups(mSelectType);
    mBackupsWidget->setObjectName(QString::fromUtf8("Backups"));
    ui->stackedWidget->addWidget(mBackupsWidget);
}

void NodeSelector::addSearch()
{
    mSearchWidget = new NodeSelectorTreeViewWidgetSearch(mSelectType);
    mSearchWidget->setObjectName(QString::fromUtf8("Search"));
    ui->stackedWidget->addWidget(mSearchWidget);
}

void NodeSelector::addRubbish()
{
    mRubbishWidget = new NodeSelectorTreeViewWidgetRubbish(mSelectType);
    mRubbishWidget->setObjectName(QString::fromUtf8("Rubbish"));
    ui->stackedWidget->addWidget(mRubbishWidget);
}
