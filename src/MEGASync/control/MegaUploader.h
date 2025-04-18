#ifndef MEGAUPLOADER_H
#define MEGAUPLOADER_H

#include "FolderTransferListener.h"
#include "megaapi.h"
#include "QTMegaTransferListener.h"
#include "TransferBatch.h"

#include <QDir>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QQueue>

class MegaUploader : public QObject
{
    Q_OBJECT

public:
    MegaUploader(mega::MegaApi *megaApi, std::shared_ptr<FolderTransferListener> _listener);
    virtual ~MegaUploader() = default;

    void upload(QString path, const QString &nodeName, std::shared_ptr<mega::MegaNode> parent, unsigned long long appDataID, const std::shared_ptr<TransferBatch> &transferBatch);

signals:
    void startingTransfers();

private:
    void startUpload(const QString& localPath, const QString& nodeName, unsigned long long appDataID, mega::MegaNode* parent, mega::MegaCancelToken* cancelToken);

    mega::MegaApi *megaApi;
    std::shared_ptr<FolderTransferListener> mFolderTransferListener;
    std::shared_ptr<mega::QTMegaTransferListener> mFolderTransferListenerDelegate;
};

#endif // MEGAUPLOADER_H
