#include "LocalOrRemoteUserMustChooseStalledIssue.h"

#include <MegaApplication.h>
#include <MegaUploader.h>
#include <TransfersModel.h>
#include <StalledIssuesUtilities.h>
#include "StatsEventHandler.h"

LocalOrRemoteUserMustChooseStalledIssue::LocalOrRemoteUserMustChooseStalledIssue(const mega::MegaSyncStall *stallIssue)
    : StalledIssue(stallIssue),
      mUploader(new MegaUploader(MegaSyncApp->getMegaApi(), nullptr))
{
}

LocalOrRemoteUserMustChooseStalledIssue::~LocalOrRemoteUserMustChooseStalledIssue()
{
    mUploader->deleteLater();
}

bool LocalOrRemoteUserMustChooseStalledIssue::autoSolveIssue()
{
    if(isSolvable())
    {
        chooseLastMTimeSide();

        if(isSolved())
        {
            MegaSyncApp->getStatsEventHandler()->sendEvent(AppStatsEvents::EventType::SI_LOCALREMOTE_SOLVED_AUTOMATICALLY);
            return true;
        }
    }

    return false;
}

void LocalOrRemoteUserMustChooseStalledIssue::chooseLastMTimeSide()
{
    if(consultLocalData()->getAttributes()->modifiedTime() >= consultCloudData()->getAttributes()->modifiedTime())
    {
        chooseLocalSide();
    }
    else
    {
        chooseRemoteSide();
    }
}

bool LocalOrRemoteUserMustChooseStalledIssue::UIShowFileAttributes() const
{
    return true;
}

bool LocalOrRemoteUserMustChooseStalledIssue::isSolvable() const
{
    //In case it is a backup, we cannot automatically solve it
    if(getSyncType() == mega::MegaSync::SyncType::TYPE_BACKUP)
    {
        return false;
    }

    if(isFile() && (consultLocalData()->getAttributes()->size() == consultCloudData()->getAttributes()->size()))
    {
        //Check names
        auto localName(QString::fromUtf8(MegaSyncApp->getMegaApi()->unescapeFsIncompatible(consultLocalData()->getFileName().toUtf8().constData())));
        auto cloudName(QString::fromUtf8(MegaSyncApp->getMegaApi()->unescapeFsIncompatible(consultCloudData()->getFileName().toUtf8().constData())));
        if(localName.compare(cloudName, Qt::CaseSensitive) == 0)
        {
            return true;
        }
    }

    return false;
}

void LocalOrRemoteUserMustChooseStalledIssue::fillIssue(const mega::MegaSyncStall *stall)
{
    StalledIssue::fillIssue(stall);

    std::shared_ptr<UploadTransferInfo> info(new UploadTransferInfo());
    //Check if transfer already exists
    if(isBeingSolvedByUpload(info))
    {
        setIsSolved(false);
    }
}

void LocalOrRemoteUserMustChooseStalledIssue::endFillingIssue()
{
    StalledIssue::endFillingIssue();

    if(isFile())
    {
        //For autosolving
        getLocalData()->getAttributes()->requestSize(nullptr, nullptr);
        getCloudData()->getAttributes()->requestSize(nullptr, nullptr);

        getLocalData()->getAttributes()->requestModifiedTime(nullptr, nullptr);
        getCloudData()->getAttributes()->requestModifiedTime(nullptr, nullptr);
    }
}

void LocalOrRemoteUserMustChooseStalledIssue::chooseLocalSide()
{
    if(getCloudData())
    {
        std::shared_ptr<UploadTransferInfo> info(new UploadTransferInfo());
        //Check if transfer already exists
        if(!isBeingSolvedByUpload(info))
        {
            std::shared_ptr<mega::MegaNode> node(getCloudData()->getNode());
            std::shared_ptr<mega::MegaNode> parentNode(MegaSyncApp->getMegaApi()->getNodeByHandle(info->parentHandle));
            if(parentNode)
            {
                bool versionsDisabled(Preferences::instance()->fileVersioningDisabled());
                StalledIssuesUtilities utilities;
                if (!versionsDisabled || utilities.removeRemoteFile(node.get()))
                {
                    //Using appDataId == 0 means that there will be no notification for this upload
                    mUploader->upload(info->localPath, info->filename, parentNode, 0, nullptr);

                    mChosenSide = ChosenSide::Local;
                    setIsSolved(false);
                }
            }
        }
    }
}

void LocalOrRemoteUserMustChooseStalledIssue::chooseRemoteSide()
{
    StalledIssuesUtilities utilities;
    auto syncId = syncIds().isEmpty() ? mega::INVALID_HANDLE : syncIds().first();
    utilities.removeLocalFile(consultLocalData()->getNativeFilePath(), syncId);

    mChosenSide = ChosenSide::Remote;
    setIsSolved(false);
}

void LocalOrRemoteUserMustChooseStalledIssue::chooseBothSides(QStringList* namesUsed)
{
    auto node(getCloudData()->getNode());
    if(node)
    {
        std::unique_ptr<mega::MegaNode> parentNode(MegaSyncApp->getMegaApi()->getParentNode(node.get()));
        if(parentNode)
        {
            mNewName = Utilities::getNonDuplicatedNodeName(node.get(), parentNode.get(), QString::fromUtf8(node->getName()), true, (*namesUsed));
            namesUsed->append(mNewName);
            bool result(false);
            QEventLoop eventLoop;
            MegaSyncApp->getMegaApi()->renameNode(node.get(),
                mNewName.toUtf8().constData(),
                new mega::OnFinishOneShot(MegaSyncApp->getMegaApi(),
                    [&eventLoop, &result](bool, const mega::MegaRequest&, const mega::MegaError& e)
                    {
                        result = e.getErrorCode() == mega::MegaError::API_OK;
                        eventLoop.quit();
                    }));
            eventLoop.exec();
            if (result)
            {
                mChosenSide = ChosenSide::Both;
                setIsSolved(false);
            }
        }
    }
}

LocalOrRemoteUserMustChooseStalledIssue::ChosenSide LocalOrRemoteUserMustChooseStalledIssue::lastModifiedSide() const
{
    if(isFile())
    {
        return consultLocalData()->getAttributes()->modifiedTime() > consultCloudData()->getAttributes()->modifiedTime()
                   ? ChosenSide::Local : ChosenSide::Remote;
    }

    return ChosenSide::None;
}

LocalOrRemoteUserMustChooseStalledIssue::ChosenSide LocalOrRemoteUserMustChooseStalledIssue::getChosenSide() const
{
    return mChosenSide;
}
