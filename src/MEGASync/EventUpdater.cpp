#include "EventUpdater.h"

#include <QApplication>

EventUpdater::EventUpdater(qsizetype _totalSize, qsizetype threshold):
    mTotalSize(_totalSize)
{
    mUpdateThreshold = (mTotalSize < threshold) ? mTotalSize : threshold;
}

bool EventUpdater::update(qsizetype currentSize)
{
    if(mUpdateThreshold > 0)
    {
        if (currentSize > 0 && currentSize % mUpdateThreshold == 0)
        {
            QApplication::processEvents();
            return true;
        }
    }

    return false;
}
