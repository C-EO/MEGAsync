#include "IgnoredStalledIssue.h"

#include <StalledIssuesUtilities.h>
#include <MegaApplication.h>
#include <syncs/control/MegaIgnoreManager.h>
#include <DialogOpener.h>
#include <StalledIssuesDialog.h>
#include <StalledIssuesModel.h>

QMap<mega::MegaHandle, bool> IgnoredStalledIssue::mSymLinksIgnoredInSyncs = QMap<mega::MegaHandle, bool>();

IgnoredStalledIssue::IgnoredStalledIssue(const mega::MegaSyncStall *stallIssue)
    : StalledIssue(stallIssue) ,
    mLinkType(mega::MegaSyncStall::SyncPathProblem::NoProblem)
{

}

void IgnoredStalledIssue::clearIgnoredSyncs()
{
    mSymLinksIgnoredInSyncs.clear();
}

mega::MegaSyncStall::SyncPathProblem IgnoredStalledIssue::linkType() const
{
    return mLinkType;
}

bool IgnoredStalledIssue::isAutoSolvable() const
{
    //Always autosolvable, we don´t need to check if smart mode is active
    return !isSolved() && !syncIds().isEmpty() &&
           (mLinkType == mega::MegaSyncStall::SyncPathProblem::DetectedSymlink ||
            mLinkType == mega::MegaSyncStall::SyncPathProblem::DetectedHardLink ||
            mLinkType == mega::MegaSyncStall::SyncPathProblem::DetectedSpecialFile);
}

void IgnoredStalledIssue::fillIssue(const mega::MegaSyncStall* stall)
{
    StalledIssue::fillIssue(stall);
    if(getReason() == mega::MegaSyncStall::FileIssue && consultLocalData())
    {
        mLinkType = consultLocalData()->getPath().pathProblem;
    }

    if(stall->couldSuggestIgnoreThisPath(false, 0))
    {
        mIgnoredPaths.append({getLocalData()->getNativeFilePath(), false});
    }
    if(stall->couldSuggestIgnoreThisPath(false, 1))
    {
        mIgnoredPaths.append({getLocalData()->getNativeFilePath(), false});
    }
    if(stall->couldSuggestIgnoreThisPath(true, 0))
    {
        mIgnoredPaths.append({getLocalData()->getNativeFilePath(), false});
    }
    if(stall->couldSuggestIgnoreThisPath(true, 1))
    {
        mIgnoredPaths.append({getLocalData()->getNativeFilePath(), false});
    }
}

bool IgnoredStalledIssue::isSymLink() const
{
    return mLinkType == mega::MegaSyncStall::SyncPathProblem::DetectedSymlink;
}

bool IgnoredStalledIssue::isSpecialLink() const
{
    return mLinkType == mega::MegaSyncStall::SyncPathProblem::DetectedSpecialFile;
}

bool IgnoredStalledIssue::isHardLink() const
{
    return mLinkType == mega::MegaSyncStall::SyncPathProblem::DetectedHardLink;
}

bool IgnoredStalledIssue::isExpandable() const
{
    return false;
}

bool IgnoredStalledIssue::checkForExternalChanges()
{
    return false;
}

//Only for Symbolic, hard and special links
bool IgnoredStalledIssue::autoSolveIssue()
{
    setAutoResolutionApplied(true);
    auto result(false);

    if(!syncIds().isEmpty())
    {
        auto syncId(firstSyncId());
        //We could do it without this static list
        //as the MegaIgnoreManager checks if the rule already exists
        //but with the list we save the megaignore parser, so it is more efficient
        if(!mSymLinksIgnoredInSyncs.contains(syncId) ||
           !isSymLink())
        {
            std::shared_ptr<mega::MegaSync>sync(MegaSyncApp->getMegaApi()->getSyncByBackupId(syncId));
            if(sync)
            {
                auto folderPath(QDir::toNativeSeparators(QString::fromUtf8(sync->getLocalFolder())));

                MegaIgnoreManager ignoreManager(folderPath, false);
                if(isSymLink())
                {
                    ignoreManager.addIgnoreSymLinksRule();
                    mSymLinksIgnoredInSyncs.insert(syncId, true);
                }
                else
                {
                    QDir dir;
                    foreach(auto ignoredPath, mIgnoredPaths)
                    {
                        if(ignoredPath.cloud)
                        {
                            dir.setPath(QString::fromUtf8(sync->getLastKnownMegaFolder()));
                        }
                        else
                        {
                            dir.setPath(folderPath);
                        }

                        ignoreManager.addNameRule(MegaIgnoreNameRule::Class::EXCLUDE,
                            dir.relativeFilePath(ignoredPath.path));
                    }
                }

                auto changesApplied(ignoreManager.applyChanges());
                if(changesApplied < MegaIgnoreManager::ApplyChangesError::NO_WRITE_PERMISSION)
                {
                    result = true;
                }
                else
                {
                    if(isSymLink())
                    {
                        mSymLinksIgnoredInSyncs.remove(syncId);
                    }
                }
            }
        }
        //Only done for sym links
        else if(mSymLinksIgnoredInSyncs.value(syncId) == true)
        {
            result = true;
        }
    }

    return result;
}
