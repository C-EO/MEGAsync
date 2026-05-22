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
}
