#ifndef STALLEDISSUEFACTORY_H
#define STALLEDISSUEFACTORY_H

#include "MultiStepIssueSolver.h"
#include "StalledIssue.h"

#include <chrono>
#include <unordered_map>

class MoveOrRenameCannotOccurFactory;

class ReceivedStalledIssues
{
public:
    ReceivedStalledIssues() = default;
    ~ReceivedStalledIssues() = default;

    bool isEmpty() const
    {
        return mAutoSolvedStalledIssues.isEmpty() && mActiveStalledIssues.isEmpty() &&
               mFailedAutoSolvedStalledIssues.isEmpty();
    }

    qsizetype size() const
    {
        return mAutoSolvedStalledIssues.size() + mActiveStalledIssues.size() +
               mFailedAutoSolvedStalledIssues.size();
    }

    void clear()
    {
        mActiveStalledIssues.clear();
        mAutoSolvedStalledIssues.clear();
        mFailedAutoSolvedStalledIssues.clear();
    }

    StalledIssuesVariantList& autoSolvedStalledIssues()
    {
        return mAutoSolvedStalledIssues;
    }

    StalledIssuesVariantList& activeStalledIssues()
    {
        return mActiveStalledIssues;
    }

    StalledIssuesVariantList& failedAutoSolvedStalledIssues()
    {
        return mFailedAutoSolvedStalledIssues;
    }

private:
    friend class StalledIssuesCreator;

    StalledIssuesVariantList mActiveStalledIssues;
    StalledIssuesVariantList mAutoSolvedStalledIssues;
    StalledIssuesVariantList mFailedAutoSolvedStalledIssues;
};

Q_DECLARE_METATYPE(ReceivedStalledIssues)

class StalledIssuesFactory
{
public:
    StalledIssuesFactory();
    virtual ~StalledIssuesFactory() = default;

    virtual std::shared_ptr<StalledIssue> createIssue(MultiStepIssueSolverBase* solver,
                                                      const mega::MegaSyncStall* stall) = 0;

    virtual void clear() {}

    virtual void finish() {}
};

enum class UpdateType
{
    NONE,
    UI,
    EVENT,
    AUTO_SOLVE
};

Q_DECLARE_METATYPE(UpdateType)

// Discard any issue received before 60 seconds after fixing it (check the issue type as not all of
// them use this feature)
static constexpr auto RECENTLY_RESOLVED_ISSUE_TTL = std::chrono::seconds(60);

class StalledIssuesCreator : public QObject
{
        Q_OBJECT
public:
    struct IssuesCount
    {
        int totalIssues = 0;
        int currentIssueBeingSolved = 0;
        int issuesFixed = 0;
        int issuesFailed = 0;

        bool isEmpty(){return totalIssues + currentIssueBeingSolved + issuesFailed + issuesFixed == 0;}
    };

    StalledIssuesCreator();

    void createIssues(const mega::MegaSyncStallMap* stallsMap, UpdateType updateType);
    void rememberResolvedIssueHash(size_t hash);

    bool multiStepIssueSolveActive() const;
    void addMultiStepIssueSolver(MultiStepIssueSolverBase* issue);

    ReceivedStalledIssues getStalledIssues() const;

    void start();
    void finish();

signals:
    void solvingIssues(StalledIssuesCreator::IssuesCount count);
    void solvingIssuesFinished(StalledIssuesCreator::IssuesCount count);

protected:
    ReceivedStalledIssues mStalledIssues;

private:
    void purgeRecentlyResolvedIssueHashes();
    bool shouldDiscardRecentlyResolvedIssue(size_t hash) const;

    QPointer<MultiStepIssueSolverBase>
        getMultiStepIssueSolverByStall(const mega::MegaSyncStall* stall, mega::MegaHandle syncId);

    QMultiMap<mega::MegaSyncStall::SyncStallReason,
        MultiStepIssueSolverBase*> mMultiStepIssueSolversByReason;
    std::shared_ptr<MoveOrRenameCannotOccurFactory> mMoveOrRenameCannotOccurFactory;
    // Keeps recently solved issue hashes until the SDK reports the updated stall state.
    std::unordered_map<size_t, std::chrono::steady_clock::time_point> mRecentlyResolvedIssueHashes;
};

Q_DECLARE_METATYPE(StalledIssuesCreator::IssuesCount)

#endif // STALLEDISSUEFACTORY_H
