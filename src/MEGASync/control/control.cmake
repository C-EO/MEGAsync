
set(DESKTOP_APP_CONTROL_HEADERS
    control/AccountStatusController.h
    control/AppState.h
    control/AppStatsEvents.h
    control/AsyncHandler.h
    control/ConnectivityChecker.h
    control/CrashHandler.h
    control/DialogOpener.h
    control/DownloadQueueController.h
    control/EmailRequester.h
    control/StatsEventHandler.h
    control/ProxyStatsEventHandler.h
    control/ExportProcessor.h
    control/FileFolderAttributes.h
    control/HTTPServer.h
    control/ImageDownloader.h
    control/IntervalExecutioner.h
    control/LinkProcessor.h
    control/LinkObject.h
    control/LoginController.h
    control/MegaDownloader.h
    control/MegaSyncLogger.h
    control/MegaUploader.h
    control/TextDecorator.h
    control/ThreadPool.h
    control/TransferBatch.h
    control/TransferRemainingTime.h
    control/UpdateTask.h
    control/UserAttributesManager.h
    control/RequestListenerManager.h
    control/SetManager.h
    control/SetTypes.h
    control/Utilities.h
    control/Version.h
    control/gzjoin.h
    control/qrcodegen.h
    control/MegaApiSynchronizedRequest.h
    control/MergeMEGAFolders.h
    control/MEGAPathCreator.h
    control/MoveToMEGABin.h
    control/Preferences/EncryptedSettings.h
    control/Preferences/EphemeralCredentials.h
    control/Preferences/Preferences.h
    control/AccountDetailsManager.h
    control/UserMessageController.h
    control/UserMessageTypes.h
)

set(DESKTOP_APP_CONTROL_SOURCES
    control/AccountStatusController.cpp
    control/AppState.cpp
    control/AppStatsEvents.cpp
    control/ConnectivityChecker.cpp
    control/CrashHandler.cpp
    control/DialogOpener.cpp
    control/DownloadQueueController.cpp
    control/EmailRequester.cpp
    control/ProxyStatsEventHandler.cpp
    control/ExportProcessor.cpp
    control/FileFolderAttributes.cpp
    control/HTTPServer.cpp
    control/ImageDownloader.cpp
    control/IntervalExecutioner.cpp
    control/LinkProcessor.cpp
    control/LinkObject.cpp
    control/LoginController.cpp
    control/MegaDownloader.cpp
    control/MegaSyncLogger.cpp
    control/MegaUploader.cpp
    control/RequestListenerManager.cpp
    control/SetManager.cpp
    control/TextDecorator.cpp
    control/ThreadPool.cpp
    control/TransferBatch.cpp
    control/TransferRemainingTime.cpp
    control/UpdateTask.cpp
    control/UserAttributesManager.cpp
    control/Utilities.cpp
    control/qrcodegen.c
    control/MergeMEGAFolders.cpp
    control/MEGAPathCreator.cpp
    control/MoveToMEGABin.cpp
    control/Preferences/EncryptedSettings.cpp
    control/Preferences/EphemeralCredentials.cpp
    control/Preferences/Preferences.cpp
    control/StatsEventHandler.cpp
    control/AccountDetailsManager.cpp
    control/UserMessageController.cpp
)

target_sources(MEGAsync
    PRIVATE
    ${DESKTOP_APP_CONTROL_HEADERS}
    ${DESKTOP_APP_CONTROL_SOURCES}
)

set (INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/Preferences
)
target_include_directories(MEGAsync PRIVATE ${INCLUDE_DIRECTORIES})
