#ifndef NODESELECTOROPERATIONTRACKER_H
#define NODESELECTOROPERATIONTRACKER_H

#include "megaapi.h"

#include <QList>
#include <QQueue>

class NodeSelectorOperationTracker
{
public:
    enum ItemCategory
    {
        NONE = 0x0,
        FILES = 0x1,
        FOLDERS = 0x2
    };

    struct FinishedRequestGroup
    {
        bool matched = false;
        int type = -1;
        bool groupFinished = false;
        QList<mega::MegaHandle> failedHandles;
        int movedItemCategories = NONE;
    };

    bool beginMoveOperation(int number);
    bool consumeMoveOperations(int number);
    void clearMoveOperations();
    bool hasMoveOperations() const;
    int pendingMoveItems() const;

    void beginRequestGroup(int type, const QList<mega::MegaHandle>& handles);
    FinishedRequestGroup finishRequest(mega::MegaHandle handle, bool failed, int movedItemCategory);
    bool hasRequestGroups() const;

private:
    struct RequestGroup
    {
        int type = -1;
        QList<mega::MegaHandle> pendingHandles;
        QList<mega::MegaHandle> failedHandles;
        int movedItemCategories = NONE;
    };

    QQueue<int> mPendingMoveOperations;
    QList<RequestGroup> mRequestGroups;
};

#endif // NODESELECTOROPERATIONTRACKER_H
