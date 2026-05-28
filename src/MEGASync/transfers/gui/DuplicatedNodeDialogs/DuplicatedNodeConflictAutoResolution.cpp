#include "DuplicatedNodeConflictAutoResolution.h"

#include "DuplicatedNodeInfo.h"
#include "Utilities.h"

#include <QHash>
#include <QStringList>

void DuplicatedNodeConflictAutoResolution::resolveFolderConflictsForCopy(
    std::shared_ptr<ConflictTypes> conflicts)
{
    if (!conflicts->isConflictFree())
    {
        QHash<mega::MegaHandle, QStringList> usedNamesByParent =
            conflicts->mReservedNodeNamesByTargetParent;

        autoRenameFolderConflictsForCopy(conflicts, conflicts->mFolderConflicts, usedNamesByParent);
        autoRenameFolderConflictsForCopy(conflicts,
                                         conflicts->mFolderNameConflicts,
                                         usedNamesByParent);
    }
}

void DuplicatedNodeConflictAutoResolution::autoRenameFolderConflictsForCopy(
    std::shared_ptr<ConflictTypes> conflicts,
    QList<std::shared_ptr<DuplicatedNodeInfo>>& folderConflicts,
    QHash<mega::MegaHandle, QStringList>& usedNamesByParent)
{
    QList<std::shared_ptr<DuplicatedNodeInfo>> unresolvedConflicts;

    for (const auto& conflict: folderConflicts)
    {
        auto moveConflict = std::dynamic_pointer_cast<DuplicatedMoveNodeInfo>(conflict);
        if (!moveConflict)
        {
            unresolvedConflicts.append(conflict);
            continue;
        }

        auto parentNode = moveConflict->getParentNode();
        auto sourceNode = moveConflict->getSourceItemNode();
        if (!parentNode || !sourceNode || !sourceNode->isFolder())
        {
            unresolvedConflicts.append(conflict);
            continue;
        }

        const auto parentHandle = parentNode->getHandle();

        auto folderName = moveConflict->getName();
        if (folderName.isEmpty())
        {
            folderName = QString::fromUtf8(sourceNode->getName());
        }

        auto& usedNames = usedNamesByParent[parentHandle];
        auto& reservedNames = conflicts->mReservedNodeNamesByTargetParent[parentHandle];

        moveConflict->setSolution(NodeItemType::UPLOAD_AND_RENAME);
        moveConflict->setNewName(Utilities::getNonDuplicatedNodeName(sourceNode.get(),
                                                                     parentNode.get(),
                                                                     folderName,
                                                                     false,
                                                                     usedNames));
        usedNames.append(moveConflict->getNewName());
        reservedNames.append(moveConflict->getNewName());
        conflicts->mResolvedConflicts.append(moveConflict);
    }

    folderConflicts = unresolvedConflicts;
}
