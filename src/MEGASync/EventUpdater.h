#ifndef EVENTUPDATER_H
#define EVENTUPDATER_H

#include <QtCore/qtypes.h>
class EventUpdater
{
public:
    EventUpdater(qsizetype _totalSize, qsizetype threshold = 100);
    virtual ~EventUpdater() = default;

    bool update(qsizetype currentSize);

private:
    qsizetype mTotalSize;
    qsizetype mUpdateThreshold;
};

#endif // EVENTUPDATER_H
