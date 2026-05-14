import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.images 1.0
import components.texts 1.0
import components.buttons 1.0
import components.menus 1.0

Item {
    id: root

    property int errorId
    property string errorMessage

    readonly property int issueHandlerSpacing: 4
    readonly property int topErrorLabelMargin: 8
    readonly property int errorLabelMargin: 10
    readonly property int issueLabelPixelSize: 12
    readonly property int buttonActionSpacing: 8
    readonly property int buttonFocusBorder: 4

    readonly property int k_LOCAL_PATH_TEMPORARY_UNAVAILABLE: 6 //
    readonly property int k_LOCAL_PATH_UNAVAILABLE: 7 //
    readonly property int k_REMOTE_NODE_NOT_FOUND: 8 //
    readonly property int k_STORAGE_OVERQUOTA: 9 //
    readonly property int k_LOCAL_FILESYSTEM_MISMATCH: 15 //
    readonly property int k_REMOTE_NODE_INSIDE_RUBBISH: 20 //
    readonly property int k_LOGGED_OUT: 26 //
    readonly property int k_SYNC_CONFIG_WRITE_FAILURE: 31 //
    readonly property int k_COULD_NOT_CREATE_IGNORE_FILE: 34 //
    readonly property int k_SYNC_CONFIG_READ_FAILURE: 35 //
    readonly property int k_UNKNOWN_DRIVE_PATH: 36 //
    readonly property int k_NOTIFICATION_SYSTEM_UNAVAILABLE: 38 //
    readonly property int k_UNABLE_TO_ADD_WATCH: 39 //
    readonly property int k_UNABLE_TO_OPEN_DATABASE: 41
    readonly property int k_INSUFFICIENT_DISK_SPACE: 42 //
    readonly property int k_FAILURE_ACCESSING_PERSISTENT_STORAGE: 43 //
    readonly property int k_MISMATCH_OF_ROOT_FSID: 44 //

    anchors.fill: parent
    implicitHeight: errorInfo.implicitHeight
    implicitWidth: errorInfo.implicitWidth

    function resetActionButtonsVisibility() {
        actionRetry.visible = false;
        actionGetMoreStorage.visible = false;
        actionSetFolderPermissions.visible = false;
        actionRemoveSyncedFolder.visible = false;
        actionEnableSync.visible = false;
        actionRestoreSyncedFolder.visible = false;
    }

    states: [
        State {
            when: (errorId == k_LOCAL_PATH_TEMPORARY_UNAVAILABLE || errorId == k_LOCAL_PATH_UNAVAILABLE ||
                   errorId == k_COULD_NOT_CREATE_IGNORE_FILE || errorId == k_NOTIFICATION_SYSTEM_UNAVAILABLE ||
                   errorId == k_UNABLE_TO_ADD_WATCH || errorId == k_INSUFFICIENT_DISK_SPACE ||
                   errorId == k_FAILURE_ACCESSING_PERSISTENT_STORAGE || errorId == k_MISMATCH_OF_ROOT_FSID)

            PropertyChanges {
                target: actionRetry
                visible: true
            }
        },
        State {
            when: errorId == k_REMOTE_NODE_NOT_FOUND

            PropertyChanges {
                target: actionRemoveSyncedFolder
                visible: true
            }
        },
        State {
            when: errorId == k_STORAGE_OVERQUOTA

            PropertyChanges {
                target: actionGetMoreStorage
                visible: true
            }
        },
        State {
            when: errorId == k_LOCAL_FILESYSTEM_MISMATCH

            PropertyChanges {
                target: actionStartNewSync
                visible: true
            }
        },
        State {
            when: errorId == k_REMOTE_NODE_INSIDE_RUBBISH

            PropertyChanges {
                target: actionRestoreSyncedFolder
                visible: true
            }
        },
        State {
            when: errorId == k_LOGGED_OUT

            PropertyChanges {
                target: actionEnableSync
                visible: true
            }
        },
        State {
            when: errorId == k_SYNC_CONFIG_WRITE_FAILURE || errorId == k_SYNC_CONFIG_READ_FAILURE ||
                  errorId == k_UNABLE_TO_OPEN_DATABASE

            PropertyChanges {
                target: actionRetry
                visible: true
            }

            PropertyChanges {
                target: actionSetFolderPermissions
                visible: true
            }
        },
        State {
            when: errorId == k_UNKNOWN_DRIVE_PATH

            PropertyChanges {
                target: actionRetry
                visible: true
            }

            PropertyChanges {
                target: actionRemoveSyncedFolder
                visible: true
            }
        }
    ]

    onStateChanged: {
        resetActionButtonsVisibility();
    }

    ColumnLayout {
        id: errorInfo

        width: parent.width
        spacing: root.issueHandlerSpacing

        Text {
            id: errorLabel

            Layout.topMargin: root.topErrorLabelMargin
            Layout.rightMargin: root.errorLabelMargin
            Layout.leftMargin: root.errorLabelMargin
            Layout.bottomMargin: root.issueHandlerSpacing
            Layout.fillWidth: true
            text: "(" + root.errorId + ") " + root.errorMessage
            font.pixelSize: root.issueLabelPixelSize
            font.weight: Font.Normal
            elide: Text.ElideRight
            color: ColorTheme.textError
        }

        RowLayout{
            id: buttonList

            spacing: root.buttonActionSpacing
            Layout.bottomMargin: root.errorLabelMargin - buttonFocusBorder
            Layout.rightMargin: root.errorLabelMargin
            Layout.leftMargin: Layout.bottomMargin

            PrimaryButton {
                id: actionRetry

                visible: false
                sizes: SmallSizes {}
                icons.source: Images.rotate_cw_small_thin_outline
                icons.position: Icon.Position.LEFT
                text: SettingsStrings.solveIssueButtonRetry

                onClicked: {
                    syncSettings.retry(index);
                }
            }

            PrimaryButton {
                id: actionGetMoreStorage

                visible: false
                sizes: SmallSizes {}
                text: SettingsStrings.solveIssueGetMoreStorage

                onClicked: {
                    syncSettings.getMoreSpace();
                }
            }

            OutlineButton {
                id: actionSetFolderPermissions

                visible: false
                sizes: SmallSizes {}
                text: SettingsStrings.solveIssueSetFolderPermissions

                onClicked: {
                    syncSettings.setFolderPermissions();
                }
            }

            PrimaryButton {
                id: actionRemoveSyncedFolder

                visible: false
                colors.background: ColorTheme.buttonError
                sizes: SmallSizes {}
                text: SettingsStrings.solveIssueRemoveSyncedFolder
                icons.source: Images.trash_small_thin_outline
                icons.position: Icon.Position.LEFT
                onClicked: {
                    syncSettings.removeSyncedFolder(index);
                }
            }

            PrimaryButton {
                id: actionEnableSync

                visible: false
                sizes: SmallSizes {}
                text: SettingsStrings.solveIssueEnableSync
                icons.source: Images.power_small_thin_outline
                icons.position: Icon.Position.LEFT
                onClicked: {
                    syncSettings.enableSync(index);
                }
            }

            PrimaryButton {
                id: actionRestoreSyncedFolder

                visible: false
                sizes: SmallSizes {}
                text: SettingsStrings.solveIssueRestoreFolder
                icons.source: Images.trash_off_small_thin_outline
                icons.position: Icon.Position.LEFT
                onClicked: {
                    syncSettings.restoreSyncedFolder(index);
                }
            }

            PrimaryButton {
                id: actionStartNewSync

                visible: false
                sizes: SmallSizes {}
                text: SettingsStrings.solveIssueStartNewSync
                icons.source: Images.sync_plus_small_thin_outline
                icons.position: Icon.Position.LEFT
                onClicked: {
                    syncSettings.addNewSync();
                }
            }
        }
    }
}