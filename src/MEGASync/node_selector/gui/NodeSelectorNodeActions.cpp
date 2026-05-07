#include "NodeSelectorNodeActions.h"

#include "DialogOpener.h"
#include "MegaApplication.h"
#include "MegaNodeNames.h"
#include "MessageDialogOpener.h"
#include "NodeSelectorModel.h"
#include "RenameNodeDialog.h"
#include "Utilities.h"

#include <QCoreApplication>
#include <QMessageBox>
#include <QPointer>

#include <memory>

NodeSelectorNodeActions::NodeSelectorNodeActions(mega::MegaApi* megaApi):
    mDialogParent(nullptr),
    mModel(nullptr),
    mMegaApi(megaApi)
{}

void NodeSelectorNodeActions::setModel(NodeSelectorModel* model)
{
    mModel = model;
}

void NodeSelectorNodeActions::setDialogParent(QWidget* dialogParent)
{
    mDialogParent = dialogParent;
}

void NodeSelectorNodeActions::renameNode(mega::MegaHandle selectedHandle) const
{
    auto node = std::unique_ptr<mega::MegaNode>(mMegaApi->getNodeByHandle(selectedHandle));
    const int access = mMegaApi->getAccess(node.get());

    if (!node || access < mega::MegaShare::ACCESS_FULL || !node->isNodeKeyDecrypted() ||
        node->isTakenDown())
    {
        return;
    }

    QPointer<RenameRemoteNodeDialog> dialog(
        new RenameRemoteNodeDialog(std::move(node), mDialogParent));
    dialog->init();
    DialogOpener::showDialog(dialog);
}

void NodeSelectorNodeActions::deleteNodes(const QList<mega::MegaHandle>& handles,
                                          bool permanently,
                                          bool showConfirmationMessageBox) const
{
    if (handles.isEmpty() || !mModel)
    {
        return;
    }

    if (showConfirmationMessageBox)
    {
        MessageDialogInfo msgInfo;
        msgInfo.parent = mDialogParent;
        msgInfo.buttons = QMessageBox::Yes | QMessageBox::No;
        msgInfo.defaultButton = QMessageBox::Yes;
        msgInfo.finishFunc =
            [model = mModel, handles, permanently](QPointer<MessageDialogResult> msg)
        {
            if (msg->result() == QMessageBox::Yes)
            {
                model->deleteNodes(handles, permanently);
            }
        };

        if (permanently)
        {
            msgInfo.descriptionText = QCoreApplication::translate("NodeSelectorTreeViewWidget",
                                                                  "You cannot undo this action");
        }
        else
        {
            msgInfo.descriptionText =
                QCoreApplication::translate("NodeSelectorTreeViewWidget",
                                            "Any shared files or folders will no longer be "
                                            "accessible to the people you shared them with. You "
                                            "can still access these items in the Rubbish bin, "
                                            "restore, and share them.");
        }

        const auto type = Utilities::getHandlesType(handles);

        if (permanently)
        {
            msgInfo.buttonsText.insert(
                QMessageBox::Yes,
                QCoreApplication::translate("NodeSelectorTreeViewWidget", "Delete"));
            msgInfo.buttonsText.insert(
                QMessageBox::No,
                QCoreApplication::translate("NodeSelectorTreeViewWidget", "Cancel"));

            if (type == Utilities::HandlesType::FILES)
            {
                msgInfo.titleText = QCoreApplication::translate(
                    "NodeSelectorTreeViewWidget",
                    "You are about to permanently delete %n file. Would you like to proceed?",
                    nullptr,
                    static_cast<int>(handles.size()));
            }
            else if (type == Utilities::HandlesType::FOLDERS)
            {
                msgInfo.titleText = QCoreApplication::translate(
                    "NodeSelectorTreeViewWidget",
                    "You are about to permanently delete %n folder. Would you like to proceed?",
                    nullptr,
                    static_cast<int>(handles.size()));
            }
            else
            {
                msgInfo.titleText = QCoreApplication::translate(
                    "NodeSelectorTreeViewWidget",
                    "You are about to permanently delete %n items. Would you like to proceed?",
                    nullptr,
                    static_cast<int>(handles.size()));
            }
        }
        else
        {
            msgInfo.buttonsText.insert(
                QMessageBox::Yes,
                QCoreApplication::translate("NodeSelectorTreeViewWidget", "Move"));
            msgInfo.buttonsText.insert(
                QMessageBox::No,
                QCoreApplication::translate("NodeSelectorTreeViewWidget", "Don\u2019t move"));

            const auto node = getNode(static_cast<mega::MegaHandle>(handles.first()));
            if (handles.size() == 1 && node)
            {
                msgInfo.titleText = QCoreApplication::translate("NodeSelectorTreeViewWidget",
                                                                "Move %1 to Rubbish bin?")
                                        .arg(MegaNodeNames::getNodeName(node.get()));
            }
            else
            {
                msgInfo.titleText = QCoreApplication::translate("NodeSelectorTreeViewWidget",
                                                                "Move %n items to Rubbish bin?",
                                                                nullptr,
                                                                static_cast<int>(handles.size()));
            }
        }

        MessageDialogOpener::warning(msgInfo);
    }
    else
    {
        mModel->deleteNodes(handles, permanently);
    }
}

void NodeSelectorNodeActions::leaveShare(const QList<mega::MegaHandle>& handles) const
{
    if (handles.isEmpty() || !mModel)
    {
        return;
    }

    MessageDialogInfo msgInfo;
    msgInfo.parent = mDialogParent;
    msgInfo.buttons = QMessageBox::Yes | QMessageBox::No;
    msgInfo.defaultButton = QMessageBox::Yes;
    msgInfo.buttonsText.insert(QMessageBox::Yes,
                               QCoreApplication::translate("NodeSelectorTreeViewWidget", "Leave"));
    msgInfo.buttonsText.insert(
        QMessageBox::No,
        QCoreApplication::translate("NodeSelectorTreeViewWidget", "Don\u2019t leave"));
    msgInfo.titleText = QCoreApplication::translate("NodeSelectorTreeViewWidget",
                                                    "Leave this shared folder?",
                                                    nullptr,
                                                    static_cast<int>(handles.size()));
    msgInfo.descriptionText = QCoreApplication::translate(
        "NodeSelectorTreeViewWidget",
        "If you leave the folder, you will not be able to see it again.",
        nullptr,
        static_cast<int>(handles.size()));
    msgInfo.finishFunc = [model = mModel, handles](QPointer<MessageDialogResult> msg)
    {
        if (msg->result() == QMessageBox::Yes)
        {
            model->deleteNodes(handles, true);
        }
    };

    MessageDialogOpener::warning(msgInfo);
}

void NodeSelectorNodeActions::exportLinks(const QList<mega::MegaHandle>& handles) const
{
    MegaSyncApp->exportNodes(handles);
}

std::shared_ptr<mega::MegaNode> NodeSelectorNodeActions::getNode(mega::MegaHandle handle) const
{
    auto node = std::shared_ptr<mega::MegaNode>(mMegaApi->getNodeByHandle(handle));

    if (!node || !node->isNodeKeyDecrypted())
    {
        return nullptr;
    }

    return node;
}
