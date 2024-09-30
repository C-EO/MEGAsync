#include "NodeSelectorSpecializations.h"

#include "NodeSelectorTreeViewWidgetSpecializations.h"
#include "SyncInfo.h"
#include "ui_NodeSelector.h"
#include <DialogOpener.h>
#include <UploadToMegaDialog.h>
#include "MegaNodeNames.h"
#include "TextDecorator.h"

#include "megaapi.h"

#include "QMegaMessageBox.h"

UploadNodeSelector::UploadNodeSelector(QWidget *parent) : NodeSelector(parent)
{
    ui->fBackups->hide();
    SelectTypeSPtr selectType = SelectTypeSPtr(new UploadType);
    mCloudDriveWidget = new NodeSelectorTreeViewWidgetCloudDrive(selectType);
    mCloudDriveWidget->setObjectName(QString::fromUtf8("CloudDrive"));
    mCloudDriveWidget->setShowEmptyView(false);
    ui->stackedWidget->addWidget(mCloudDriveWidget);
    mIncomingSharesWidget = new NodeSelectorTreeViewWidgetIncomingShares(selectType);
    mIncomingSharesWidget->setObjectName(QString::fromUtf8("IncomingShares"));
    ui->stackedWidget->addWidget(mIncomingSharesWidget);
    makeConnections(selectType);
}

void UploadNodeSelector::checkSelection()
{
    auto node = getSelectedNode();
    if(node)
    {
        int access = Utilities::getNodeAccess(node->getHandle());
        if (access < mega::MegaShare::ACCESS_READWRITE)
        {
            QMegaMessageBox::MessageBoxInfo msgInfo;
            msgInfo.parent = this;
            msgInfo.title = QMegaMessageBox::errorTitle();
            msgInfo.text = tr("You need Read & Write or Full access rights to be able to upload to the selected folder.");
            msgInfo.finishFunc = [this](QPointer<QMessageBox> msg){
                reject();
            };
            QMegaMessageBox::warning(msgInfo);
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

DownloadNodeSelector::DownloadNodeSelector(QWidget *parent) : NodeSelector(parent)
{
    setWindowTitle(tr("Download"));
    SelectTypeSPtr selectType = SelectTypeSPtr(new DownloadType);
    mCloudDriveWidget = new NodeSelectorTreeViewWidgetCloudDrive(selectType);
    mCloudDriveWidget->setObjectName(QString::fromUtf8("CloudDrive"));
    ui->stackedWidget->addWidget(mCloudDriveWidget);
    mIncomingSharesWidget = new NodeSelectorTreeViewWidgetIncomingShares(selectType);
    mIncomingSharesWidget->setObjectName(QString::fromUtf8("IncomingShares"));
    ui->stackedWidget->addWidget(mIncomingSharesWidget);
    mBackupsWidget = new NodeSelectorTreeViewWidgetBackups(selectType);
    mBackupsWidget->setObjectName(QString::fromUtf8("Backups"));
    ui->stackedWidget->addWidget(mBackupsWidget);
    makeConnections(selectType);
}

void DownloadNodeSelector::checkSelection()
{
    QList<mega::MegaHandle> nodes = getMultiSelectionNodeHandle();
    int wrongNodes(0);
    foreach(auto& nodeHandle, nodes)
    {
        auto node = std::unique_ptr<mega::MegaNode>(mMegaApi->getNodeByHandle(nodeHandle));
        if(!node)
        {
            ++wrongNodes;
        }
    }

    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.parent = this;
    msgInfo.title = QMegaMessageBox::errorTitle();
    msgInfo.finishFunc = [this](QPointer<QMessageBox> msg){
        reject();
    };

    if(wrongNodes == nodes.size())
    {
        if(ui->stackedWidget->currentIndex() == CLOUD_DRIVE)
        {
            msgInfo.text = tr("The item you selected has been removed. To reselect, close this window and try again.", "", wrongNodes);
            QMegaMessageBox::warning(msgInfo);
        }
        else
        {
            msgInfo.text = tr("You no longer have access to this item. Ask the owner to share again.", "", wrongNodes);
            QMegaMessageBox::warning(msgInfo);
        }
    }
    else if(wrongNodes > 0)
    {
        QString warningMsg1 = tr("%1 item selected", "", nodes.size()).arg(nodes.size());
        msgInfo.text = tr("%1. %2 has been removed. To reselect, close this window and try again.", "", wrongNodes).arg(warningMsg1).arg(wrongNodes);
        QMegaMessageBox::warning(msgInfo);
    }
    else
    {
        accept();
    }
}

SyncNodeSelector::SyncNodeSelector(QWidget *parent) : NodeSelector(parent)
{
    ui->fBackups->hide();
    SelectTypeSPtr selectType = SelectTypeSPtr(new SyncType);
    mCloudDriveWidget = new NodeSelectorTreeViewWidgetCloudDrive(selectType);
    mCloudDriveWidget->setObjectName(QString::fromUtf8("CloudDrive"));
    mCloudDriveWidget->setShowEmptyView(false);
    ui->stackedWidget->addWidget(mCloudDriveWidget);
    mIncomingSharesWidget = new NodeSelectorTreeViewWidgetIncomingShares(selectType);
    mIncomingSharesWidget->setObjectName(QString::fromUtf8("IncomingShares"));
    ui->stackedWidget->addWidget(mIncomingSharesWidget);
    makeConnections(selectType);

    if (isFullSync())
    {
        ui->fCloudDrive->setVisible(false);
        emit ui->bShowIncomingShares->clicked();
    }
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

void SyncNodeSelector::checkSelection()
{
    auto node = getSelectedNode();
    if(node)
    {
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.parent = this;
        msgInfo.title = QMegaMessageBox::errorTitle();
        msgInfo.finishFunc = [this](QPointer<QMessageBox> msg){
            reject();
        };

        int access = Utilities::getNodeAccess(node->getHandle());
        if (access < mega::MegaShare::ACCESS_FULL)
        {
            msgInfo.text = tr("You need Full access right to be able to sync the selected folder.");
            QMegaMessageBox::warning(msgInfo);
        }
        else
        {
            std::unique_ptr<char[]>path(mMegaApi->getNodePath(node.get()));
            auto check = std::unique_ptr<mega::MegaNode>(mMegaApi->getNodeByPath(path.get()));
            if (!check)
            {
                msgInfo.text = tr("Invalid folder for synchronization.\n"
                                  "Please, ensure that you don't use characters like '\\' '/' or ':' in your folder names.");
                QMegaMessageBox::warning(msgInfo);
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

StreamNodeSelector::StreamNodeSelector(QWidget *parent) : NodeSelector(parent)
{
    SelectTypeSPtr selectType = SelectTypeSPtr(new StreamType);
    mCloudDriveWidget = new NodeSelectorTreeViewWidgetCloudDrive(selectType);
    mCloudDriveWidget->setObjectName(QString::fromUtf8("CloudDrive"));
    ui->stackedWidget->addWidget(mCloudDriveWidget);
    mIncomingSharesWidget = new NodeSelectorTreeViewWidgetIncomingShares(selectType);
    mIncomingSharesWidget->setObjectName(QString::fromUtf8("IncomingShares"));
    ui->stackedWidget->addWidget(mIncomingSharesWidget);
    mBackupsWidget = new NodeSelectorTreeViewWidgetBackups(selectType);
    mBackupsWidget->setObjectName(QString::fromUtf8("Backups"));
    ui->stackedWidget->addWidget(mBackupsWidget);
    makeConnections(selectType);
}

void StreamNodeSelector::checkSelection()
{
    auto node = getSelectedNode();
    if(node)
    {
        if (node->isFolder())
        {
            QMegaMessageBox::MessageBoxInfo msgInfo;
            msgInfo.parent = this;
            msgInfo.title = QMegaMessageBox::errorTitle();
            msgInfo.text = tr("Only files can be used for streaming.");
            msgInfo.finishFunc = [this](QPointer<QMessageBox>)
            {
                reject();
            };
            QMegaMessageBox::warning(msgInfo);
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
CloudDriveNodeSelector::CloudDriveNodeSelector(QWidget *parent) : NodeSelector(parent)
{
    mDragBackDrop = new QWidget(this);
    mDragBackDrop->hide();

    setWindowTitle(MegaNodeNames::getCloudDriveName());
    SelectTypeSPtr selectType = SelectTypeSPtr(new CloudDriveType);
    mCloudDriveWidget = new NodeSelectorTreeViewWidgetCloudDrive(selectType);
    mCloudDriveWidget->setObjectName(QString::fromUtf8("CloudDrive"));
    ui->stackedWidget->addWidget(mCloudDriveWidget);
    mIncomingSharesWidget = new NodeSelectorTreeViewWidgetIncomingShares(selectType);
    mIncomingSharesWidget->setObjectName(QString::fromUtf8("IncomingShares"));
    ui->stackedWidget->addWidget(mIncomingSharesWidget);
    mBackupsWidget = new NodeSelectorTreeViewWidgetBackups(selectType);
    mBackupsWidget->setObjectName(QString::fromUtf8("Backups"));
    ui->stackedWidget->addWidget(mBackupsWidget);
    mRubbishWidget = new NodeSelectorTreeViewWidgetRubbish(selectType);
    mRubbishWidget->setObjectName(QString::fromUtf8("Rubbish"));
    ui->stackedWidget->addWidget(mRubbishWidget);
    ui->fRubbish->show();
    makeConnections(selectType);
    resize(1280,800);
    setAcceptDrops(true);


#ifndef Q_OS_MACOS
    Qt::WindowFlags flags =  Qt::Window;
    this->setWindowFlags(flags);
#ifdef Q_OS_LINUX
    this->setWindowFlags(this->windowFlags() | (Qt::Tool));
#endif
#endif
}

void CloudDriveNodeSelector::enableDragAndDrop(bool enable)
{
    mCloudDriveWidget->enableDragAndDrop(enable);
    mRubbishWidget->enableDragAndDrop(true);
    mIncomingSharesWidget->enableDragAndDrop(true);
}

void CloudDriveNodeSelector::onCustomBottomButtonClicked(uint id)
{
    if(id == CloudDriveType::Upload)
    {
        auto selectedNode = getSelectedNode();
        if(selectedNode)
        {
            MegaSyncApp->runUploadActionWithTargetHandle(selectedNode->getHandle(), this);
        }
        else
        {
            showNotFoundNodeMessageBox();
        }
    }
    else if(id == CloudDriveType::Download)
    {
        MegaSyncApp->downloadACtionClickedWithHandles(getMultiSelectionNodeHandle());
    }
    else if(id == CloudDriveType::ClearRubbish)
    {
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.parent = this;
        msgInfo.title = QMegaMessageBox::errorTitle();
        msgInfo.buttons = QMessageBox::Yes | QMessageBox::No;
        QMap<QMessageBox::Button, QString> textsByButton;
        textsByButton.insert(QMessageBox::Yes, tr("Empty"));
        textsByButton.insert(QMessageBox::No, tr("Cancel"));
        msgInfo.buttonsText = textsByButton;
        msgInfo.text = tr("Empty Rubbish bin?");
        Text::Bold bold;
        Text::Decorator dec(&bold);
        msgInfo.informativeText = tr("All items will be permanently deleted. This action can [B]not[/B] be undone");
        dec.process(msgInfo.informativeText);
        msgInfo.finishFunc = [](QPointer<QMessageBox> msg){
            if (msg->result() == QMessageBox::Yes)
            {
                MegaSyncApp->getMegaApi()->cleanRubbishBin();
            }
        };
        QMegaMessageBox::warning(msgInfo);
    }
}

////////////////////////////////
MoveBackupNodeSelector::MoveBackupNodeSelector(QWidget *parent) : NodeSelector(parent)
{
    ui->fBackups->hide();
    ui->fIncomingShares->hide();
    SelectTypeSPtr selectType = SelectTypeSPtr(new MoveBackupType);
    mCloudDriveWidget = new NodeSelectorTreeViewWidgetCloudDrive(selectType);
    mCloudDriveWidget->setObjectName(QString::fromUtf8("CloudDrive"));
    mCloudDriveWidget->setShowEmptyView(false);
    ui->stackedWidget->addWidget(mCloudDriveWidget);
    makeConnections(selectType);
}
void MoveBackupNodeSelector::checkSelection()
{
    accept();
}
