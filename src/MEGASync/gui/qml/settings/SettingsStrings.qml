pragma Singleton
import QtQuick 2.15

QtObject {
    id: root

    readonly property string cloudDriveLabel: qsTr("Cloud Drive")
    readonly property string backupsLabel: qsTr("Backups")
    readonly property string versionsLabel: qsTr("File versions")
    readonly property string rubbishBinLabel: qsTr("Rubbish Bin")
    readonly property string downloadsLabel: qsTr("Transfers")
    readonly property string storageSpace: qsTr("Storage Space")
    readonly property string transferQuota: qsTr("Transfers")
    readonly property string yourMegaAccountIsFull:
        qsTr("Your MEGA account is full")
    readonly property string yourMegaAccountIsNearlyFull:
        qsTr("Your MEGA account is nearly full")
    readonly property string uploadsDisabledDescription:
        qsTr("Uploads are disabled and folder synchronisation is paused.")
    readonly property string nearlyFullDescription:
        qsTr("Consider upgrading to avoid interruptions to uploads and synchronisation.")
    readonly property string buyMoreStorage: qsTr("Buy more storage")
    readonly property string cloudDriveTooltipFormat: qsTr("Cloud Drive[BR]%1")
    readonly property string backupsTooltipFormat: qsTr("Backups[BR]%1")
    readonly property string versionsTooltipFormat: qsTr("File versions[BR]%1")
    readonly property string availableTooltipFormat: qsTr("Available[BR]%1")
    readonly property string rubbishBinTooltipFormat: qsTr("Rubbish Bin[BR]%1")
    readonly property string downloadsTooltipFormat: qsTr("Transfers[BR]%1")
    readonly property string tableSyncsNameColumn: qsTr("Synced folders")
    readonly property string tableSyncsStatusColumn: qsTr("Status")
    readonly property string addSync: qsTr("Add sync")
    readonly property string titleNoSync: qsTr("No syncs set up")
    readonly property string descriptionNoSync: qsTr("Add a sync to keep folders up to date ")
    readonly property string syncStateLoading: qsTr("Loading")
    readonly property string syncStatePaused: qsTr("Paused")
    readonly property string syncStateDisabled: qsTr("Disabled")
    readonly property string syncStateScanning: qsTr("Scanning")
    readonly property string syncStateSyncing: qsTr("Syncing")
    readonly property string syncStateSynced: qsTr("Synced")
    readonly property string syncIssueTitle: qsTr("Automatic sync issue resolution")
    readonly property string syncIssueDescription: qsTr("MEGA automatically detects and resolves sync issues for you. Turn it off if you prefer to review and handle them manually. [A]Learn more[/A]")
    readonly property string menuActionsSolveIssues: qsTr("Solve issues")
    readonly property string menuActionsShowInFolder: qsTr("Show in folder")
    readonly property string menuActionsOpenInMega: qsTr("Open in mega")
    readonly property string menuActionsPause: qsTr("Pause")
    readonly property string menuActionsResume: qsTr("Resume")
    readonly property string menuActionsManageExclusions: qsTr("Manage exlusions")
    readonly property string menuActionsRescan: qsTr("Rescan")
    readonly property string menuActionsReboot: qsTr("Reboot")
    readonly property string menuActionsRemoveSyncedFolder: qsTr("Remove synced folder")
    readonly property string solveIssueButtonRetry: qsTr("Retry")
    readonly property string solveIssueGetMoreStorage: qsTr("Get more storage")
    readonly property string solveIssueSetFolderPermissions: qsTr("Folder permissions")
    readonly property string solveIssueRemoveSyncedFolder: qsTr("Remove synced folder")
    readonly property string solveIssueEnableSync: qsTr("Enable sync")
    readonly property string solveIssueRestoreFolder: qsTr("Restore folder")
    readonly property string solveIssueStartNewSync: qsTr("Start new sync")
}
