#include "MegaUploader.h"

#include <QDir>
#include <QFileInfo>

MegaUploader::MegaUploader(mega::MegaApi* megaApi,
                           std::shared_ptr<FolderTransferListener> listener):
    mMegaApi(megaApi),
    mFolderTransferListener(std::move(listener)),
    mFolderTransferListenerDelegate(
        std::make_unique<mega::QTMegaTransferListener>(megaApi, mFolderTransferListener.get()))
{}

void MegaUploader::upload(std::unique_ptr<UploadInfo> uploadInfo)
{
    const QFileInfo fileInfo(uploadInfo->mLocalPath);
    const auto filePath = QDir::toNativeSeparators(fileInfo.absoluteFilePath());

    const auto appDataId =
        uploadInfo->mTransferBatch ? uploadInfo->mTransferBatch->getAppDataId() : 0;

    const auto msg = QString::fromLatin1("Starting upload : '%1' - '%2' - '%3'")
                         .arg(fileInfo.fileName(), filePath)
                         .arg(appDataId);
    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG, msg.toUtf8().constData());

    auto* remoteNode = uploadInfo->mRemoteNode.get();
    if (!remoteNode)
    {
        remoteNode = mMegaApi->getNodeByHandle(uploadInfo->mRemoteNodeHandle);
    }

    startUpload(filePath,
                uploadInfo->mNodeName,
                appDataId,
                remoteNode,
                uploadInfo->mTransferBatch ? uploadInfo->mTransferBatch->getCancelTokenPtr() :
                                             nullptr,
                uploadInfo->mPiTagTrigger);

    emit startingTransfers();
}

void MegaUploader::startUpload(const QString& localPath,
                               const QString& nodeName,
                               AppDataID appDataID,
                               mega::MegaNode* parent,
                               mega::MegaCancelToken* cancelToken,
                               PiTagTrigger piTagTrigger)
{
    const auto appData =
        appDataID > 0 ? (QString::number(appDataID) + QLatin1Char('*')).toUtf8() : QByteArray();
    mega::MegaUploadOptions options;
    options.fileName = nodeName.toStdString();
    options.appData = appData.isEmpty() ? nullptr : appData.constData();
    options.pitagTrigger = piTagTrigger;

    mMegaApi->startUpload(localPath.toUtf8().constData(),
                          parent,
                          cancelToken,
                          &options,
                          mFolderTransferListenerDelegate.get());
}
