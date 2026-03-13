#include "TransferBatch.h"

#include "TransferMetaData.h"

#include <QDir>

/*************************/
/*** TransferBatch *******/
/*************************/

TransferBatch::TransferBatch(AppDataID appDataId):
    mHasFinished(false),
    mAppDataId(appDataId)
{
    mCancelToken = std::shared_ptr<mega::MegaCancelToken>(mega::MegaCancelToken::createInstance());
}

bool TransferBatch::isEmpty()
{
    return mHasFinished;
}

void TransferBatch::cancel()
{
    auto data = TransferMetaDataContainer::getAppDataById(mAppDataId);
    if(data)
    {
        data->processCancelled();
    }

    mCancelToken->cancel();
}

void TransferBatch::onScanCompleted(AppDataID appDataId)
{
    mHasFinished = appDataId == mAppDataId;
}

QString TransferBatch::description()
{
    auto data = TransferMetaDataContainer::getAppDataById(mAppDataId);
    if(data)
    {
        return QString::fromLatin1("%1 nodes").arg(data->getPendingFiles());
    }

    return QString();
}

mega::MegaCancelToken* TransferBatch::getCancelTokenPtr()
{
    return mCancelToken.get();
}

std::shared_ptr<mega::MegaCancelToken> TransferBatch::getCancelToken()
{
    return mCancelToken;
}

AppDataID TransferBatch::getAppDataId() const
{
    return mAppDataId;
}

/*************************/
/*** BlockingBatch *******/
/*************************/

BlockingBatch::~BlockingBatch()
{
    clearBatch();
}

void BlockingBatch::add(std::shared_ptr<TransferBatch> _batch)
{
    mBatch = _batch;
}

void BlockingBatch::removeBatch()
{
    mBatch.reset();
}

void BlockingBatch::cancelTransfer()
{
    if (isValid())
    {
        mBatch->cancel();
        cancelled = true;
    }
}

void BlockingBatch::onScanCompleted(AppDataID appDataId)
{
    if (isValid())
    {
        mBatch->onScanCompleted(appDataId);
        if (mBatch->isEmpty())
        {
            clearBatch();
        }
    }
}

bool BlockingBatch::isBlockingStageFinished()
{
    if (isValid())
    {
        return mBatch->isEmpty();
    }
    return true;
}

void BlockingBatch::setAsUnblocked()
{
    clearBatch();
}

bool BlockingBatch::hasCancelToken()
{
    return mBatch && mBatch->getCancelTokenPtr();
}

bool BlockingBatch::isValid() const
{
    return mBatch != nullptr;
}

bool BlockingBatch::isCancelled() const
{
    return cancelled;
}

bool BlockingBatch::hasNodes() const
{
    return (isValid()) ? !mBatch->isEmpty() : false;
}

std::shared_ptr<mega::MegaCancelToken> BlockingBatch::getCancelToken()
{
    if (isValid())
    {
        return mBatch->getCancelToken();
    }
    return std::shared_ptr<mega::MegaCancelToken>();
}

QString BlockingBatch::description()
{
    if (isValid())
    {
        return mBatch->description();
    }
    return QString::fromLatin1("nullptr");
}

void BlockingBatch::clearBatch()
{
   mBatch = nullptr;
   cancelled = false;
}
