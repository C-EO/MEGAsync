#ifndef MEGAUPLOADER_H
#define MEGAUPLOADER_H

#include "FolderTransferListener.h"
#include "megaapi.h"
#include "QTMegaTransferListener.h"
#include "TransferBatch.h"

#include <memory>

using AppDataID = unsigned long long;
using PiTagTrigger = decltype(mega::MegaUploadOptions::pitagTrigger);

class MegaUploader: public QObject
{
    Q_OBJECT

public:
    class UploadInfo
    {
    public:
        UploadInfo(PiTagTrigger piTagTrigger):
            mRemoteNodeHandle{mega::INVALID_HANDLE},
            mPiTagTrigger(piTagTrigger)
        {}

        QString mLocalPath;
        QString mNodeName;
        std::shared_ptr<mega::MegaNode> mRemoteNode;
        mega::MegaHandle mRemoteNodeHandle;
        std::shared_ptr<TransferBatch> mTransferBatch;
        PiTagTrigger mPiTagTrigger;
    };

    MegaUploader(mega::MegaApi* megaApi, std::shared_ptr<FolderTransferListener> listener);
    virtual ~MegaUploader() = default;

    void upload(std::unique_ptr<UploadInfo> uploadInfo);

signals:
    void startingTransfers();

private:
    void startUpload(const QString& localPath,
                     const QString& nodeName,
                     AppDataID appDataID,
                     mega::MegaNode* parent,
                     mega::MegaCancelToken* cancelToken,
                     PiTagTrigger piTagTrigger);

    mega::MegaApi* mMegaApi;
    std::shared_ptr<FolderTransferListener> mFolderTransferListener;
    std::unique_ptr<mega::QTMegaTransferListener> mFolderTransferListenerDelegate;
};

#endif // MEGAUPLOADER_H
