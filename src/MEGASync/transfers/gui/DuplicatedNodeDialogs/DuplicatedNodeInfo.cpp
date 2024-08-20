#include "DuplicatedNodeInfo.h"
#include "DuplicatedUploadChecker.h"

#include <Utilities.h>
#include <MegaApplication.h>
#include <EventUpdater.h>

DuplicatedNodeInfo::DuplicatedNodeInfo(DuplicatedUploadBase* checker)
    : mSolution(NodeItemType::UPLOAD),
    mSourceItemIsFile(false),
    mHasConflict(false),
    mIsNameConflict(false),
    mChecker(checker)
{
}

//UPLOAD INFO
const std::shared_ptr<mega::MegaNode> &DuplicatedNodeInfo::getParentNode() const
{
    return mParentNode;
}

void DuplicatedNodeInfo::setParentNode(const std::shared_ptr<mega::MegaNode> &newParentNode)
{
    mParentNode = newParentNode;
}

const std::shared_ptr<mega::MegaNode> &DuplicatedNodeInfo::getConflictNode() const
{
    return mConflictNode;
}

void DuplicatedNodeInfo::setConflictNode(const std::shared_ptr<mega::MegaNode> &newRemoteConflictNode)
{
    mConflictNode = newRemoteConflictNode;

    auto time = newRemoteConflictNode->isFile() ? mConflictNode->getModificationTime()
                                                : mConflictNode->getCreationTime();
    mConflictNodeModifiedTime = QDateTime::fromSecsSinceEpoch(time);
}

const QString &DuplicatedNodeInfo::getSourceItemPath() const
{
    return mSourcePath;
}

void DuplicatedNodeInfo::setSourceItemPath(const QString &newLocalPath)
{
    mSourcePath = newLocalPath;

    QFileInfo localNode(mSourcePath);
    mSourceItemIsFile = localNode.exists() && localNode.isFile();
}

NodeItemType DuplicatedNodeInfo::getSolution() const
{
    return mSolution;
}

void DuplicatedNodeInfo::setSolution(NodeItemType newSolution)
{
    mSolution = newSolution;
}

const QString &DuplicatedNodeInfo::getNewName()
{
    if(mSolution == NodeItemType::UPLOAD_AND_RENAME)
    {
        if(mNewName.isEmpty() && mConflictNode)
        {
            mNewName = Utilities::getNonDuplicatedNodeName(mConflictNode.get(), mParentNode.get(), mName, false, mChecker->getCheckedNames());
            auto& checkedNames = mChecker->getCheckedNames();
            checkedNames.removeOne(mName);
            checkedNames.append(mNewName);
        }
    }

    return mNewName;
}

const QString &DuplicatedNodeInfo::getDisplayNewName()
{
    if(mDisplayNewName.isEmpty())
    {
        mDisplayNewName = Utilities::getNonDuplicatedNodeName(mConflictNode.get(), mParentNode.get(), mName, false, mChecker->getCheckedNames());
    }

    return mDisplayNewName;
}

const QString &DuplicatedNodeInfo::getName() const
{
    return mName;
}

bool DuplicatedNodeInfo::hasConflict() const
{
    return mHasConflict;
}

void DuplicatedNodeInfo::setHasConflict(bool newHasConflict)
{
    mHasConflict = newHasConflict;
}

bool DuplicatedNodeInfo::sourceItemIsFile() const
{
    return mSourceItemIsFile;
}

bool DuplicatedNodeInfo::conflictNodeIsFile() const
{
    return mConflictNode->isFile();
}

const QDateTime &DuplicatedNodeInfo::getNodeModifiedTime() const
{
    return mConflictNodeModifiedTime;
}

const QDateTime &DuplicatedNodeInfo::getSourceItemModifiedTime() const
{
    return mSourceItemModifiedTime;
}

void DuplicatedNodeInfo::setSourceItemModifiedTime(const QDateTime &newSourceItemModifiedTime)
{
    mSourceItemModifiedTime = newSourceItemModifiedTime;
}

bool DuplicatedNodeInfo::haveDifferentType() const
{
    return sourceItemIsFile() != conflictNodeIsFile();
}

bool DuplicatedNodeInfo::isNameConflict() const
{
    return mIsNameConflict;
}

void DuplicatedNodeInfo::setIsNameConflict(bool newIsNameConflict)
{
    mIsNameConflict = newIsNameConflict;
}

DuplicatedUploadBase* DuplicatedNodeInfo::checker() const
{
    return mChecker;
}

void DuplicatedNodeInfo::setNewName(const QString &newNewName)
{
    mNewName = newNewName;
}

void DuplicatedNodeInfo::setName(const QString &newName)
{
    mName = newName;
}

mega::MegaHandle DuplicatedMoveNodeInfo::getSourceItemHandle() const
{
    return mSourceItemNode->getHandle();
}

void DuplicatedMoveNodeInfo::setSourceItemHandle(const mega::MegaHandle& sourceItemHandle)
{
    mSourceItemNode = std::shared_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getNodeByHandle(sourceItemHandle));
}

bool DuplicatedMoveNodeInfo::sourceItemIsFile() const
{
    return mSourceItemNode->isFile();
}

std::shared_ptr<ConflictTypes>
    CheckDuplicatedNodes::checkMoves(QList<mega::MegaHandle> moveHandles,
                                     std::shared_ptr<mega::MegaNode> sourceNode,
                                     std::shared_ptr<mega::MegaNode> targetNode)
{
    auto conflicts = std::make_shared<ConflictTypes>();
    conflicts->mFolderCheck = new DuplicatedMoveFolder();
    conflicts->mFileCheck = new DuplicatedMoveFile();
    conflicts->mSourceNode = sourceNode;
    conflicts->mTargetNode = targetNode;

    std::unique_ptr<mega::MegaNodeList> nodes(
        MegaSyncApp->getMegaApi()->getChildren(targetNode.get()));
    QHash<QString, mega::MegaNode*> nodesOnCloudDrive;

    for (int index = 0; index < nodes->size(); ++index)
    {
        QString nodeName(QString::fromUtf8(nodes->get(index)->getName()));
        nodesOnCloudDrive.insert(nodeName, nodes->get(index));
    }

    auto counter(0);
    EventUpdater checkUpdater(moveHandles.size());

    while (!moveHandles.isEmpty())
    {
        auto moveHandle(moveHandles.takeFirst());
        std::unique_ptr<mega::MegaNode> moveNode(
            MegaSyncApp->getMegaApi()->getNodeByHandle(moveHandle));
        auto moveNodeName(QString::fromUtf8(moveNode->getName()));

        DuplicatedUploadBase* checker(nullptr);
        if (moveNode->isFile())
        {
            checker = conflicts->mFileCheck;
        }
        else
        {
            checker = conflicts->mFolderCheck;
        }

        auto info = std::make_shared<DuplicatedMoveNodeInfo>(checker);
        info->setParentNode(targetNode);
        info->setSourceItemHandle(moveHandle);

        auto MEGANode(nodesOnCloudDrive.value(moveNodeName));
        if (MEGANode)
        {
            std::shared_ptr<mega::MegaNode> smartNode(MEGANode->copy());
            info->setConflictNode(smartNode);
            info->setHasConflict(true);
            info->setName(moveNodeName);
            info->setIsNameConflict(true);
            moveNode->isFile() ? conflicts->mFileNameConflicts.append(info) :
                                 conflicts->mFolderNameConflicts.append(info);
        }
        else
        {
            conflicts->mResolvedConflicts.append(info);
        }

        checkUpdater.update(counter);
        counter++;
    }

    return conflicts;
}

std::shared_ptr<ConflictTypes>
    CheckDuplicatedNodes::checkUploads(QQueue<QString>& nodePaths,
                                       std::shared_ptr<mega::MegaNode> targetNode)
{
    auto conflicts = std::make_shared<ConflictTypes>();
    conflicts->mFolderCheck = new DuplicatedUploadFolder();
    conflicts->mFileCheck = new DuplicatedUploadFile();
    conflicts->mTargetNode = targetNode;

    std::unique_ptr<mega::MegaNodeList> nodes(
        MegaSyncApp->getMegaApi()->getChildren(targetNode.get()));
    QHash<QString, mega::MegaNode*> nodesOnCloudDrive;

    for (int index = 0; index < nodes->size(); ++index)
    {
        QString nodeName(QString::fromUtf8(nodes->get(index)->getName()));
        nodesOnCloudDrive.insert(nodeName.toLower(), nodes->get(index));
    }

    auto counter(0);
    EventUpdater checkUpdater(nodePaths.size());

    while (!nodePaths.isEmpty())
    {
        auto localPath(nodePaths.dequeue());
        QFileInfo localPathInfo(localPath);
        bool isFile(localPathInfo.isFile());
        DuplicatedUploadBase* checker(nullptr);
        if (isFile)
        {
            checker = conflicts->mFileCheck;
        }
        else
        {
            checker = conflicts->mFolderCheck;
        }

        auto info = std::make_shared<DuplicatedNodeInfo>(checker);
        info->setSourceItemPath(localPathInfo.absoluteFilePath());
        info->setParentNode(targetNode);

        QString nodeToUploadName(localPathInfo.fileName());
        auto node(nodesOnCloudDrive.value(nodeToUploadName.toLower()));
        if (node)
        {
            std::shared_ptr<mega::MegaNode> smartNode(node->copy());
            info->setConflictNode(smartNode);
            info->setHasConflict(true);
            info->setName(nodeToUploadName);

            auto nodeName(QString::fromUtf8(node->getName()));
            if (nodeName.compare(nodeToUploadName) != 0)
            {
                info->setIsNameConflict(true);
                isFile ? conflicts->mFileNameConflicts.append(info) :
                         conflicts->mFolderNameConflicts.append(info);
            }
            else
            {
                isFile ? conflicts->mFileConflicts.append(info) :
                         conflicts->mFolderConflicts.append(info);
            }
        }
        else
        {
            conflicts->mResolvedConflicts.append(info);
        }

        checkUpdater.update(counter);
        counter++;
    }

    return conflicts;
}

bool ConflictTypes::isEmpty() const
{
    return mFileConflicts.isEmpty() && mFolderConflicts.isEmpty() && mFileNameConflicts.isEmpty() &&
           mFolderNameConflicts.isEmpty();
}
