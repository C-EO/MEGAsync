#include "RestoreNodeManager.h"

#include "MegaApplication.h"
#include "NodeSelectorModel.h"

RestoreNodeManager::RestoreNodeManager(NodeSelectorModel* model, QObject* parent):
    QObject(parent),
    mModel(model)
{}

void RestoreNodeManager::onRestoreClicked(const QList<mega::MegaHandle>& handles)
{
    if (mModel)
    {
        mRestoredItems = handles;

        QList<QPair<mega::MegaHandle, std::shared_ptr<mega::MegaNode>>> moveHandles;

        foreach(auto handle, handles)
        {
            auto node =
                std::shared_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getNodeByHandle(handle));
            if (node)
            {
                auto newParent = std::shared_ptr<mega::MegaNode>(
                    MegaSyncApp->getMegaApi()->getNodeByHandle(node->getRestoreHandle()));
                QPair<mega::MegaHandle, std::shared_ptr<mega::MegaNode>> p{handle, newParent};
                moveHandles.append(p);
            }
        }

        mModel->processNodesAndCheckConflicts(moveHandles,
                                              MegaSyncApp->getRubbishNode(),
                                              MoveActionType::RESTORE);
    }
}
