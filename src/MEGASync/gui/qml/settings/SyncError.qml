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
    property string syncLocalFolder

    readonly property int issueHandlerSpacing: 4
    readonly property int topErrorLabelMargin: 8
    readonly property int errorLabelMargin: 10
    readonly property int issueLabelPixelSize: 12
    readonly property int buttonActionSpacing: 0
    readonly property int buttonFocusBorder: 4
    readonly property int timeToResetActionButtonState: 5000

    // error codes
    readonly property int k_LOCAL_PATH_TEMPORARY_UNAVAILABLE: 6
    readonly property int k_LOCAL_PATH_UNAVAILABLE: 7
    readonly property int k_REMOTE_NODE_NOT_FOUND: 8
    readonly property int k_STORAGE_OVERQUOTA: 9
    readonly property int k_LOCAL_FILESYSTEM_MISMATCH: 15
    readonly property int k_REMOTE_NODE_MOVED_TO_RUBBISH: 19
    readonly property int k_REMOTE_NODE_INSIDE_RUBBISH: 20
    readonly property int k_LOGGED_OUT: 26
    readonly property int k_SYNC_CONFIG_WRITE_FAILURE: 31
    readonly property int k_COULD_NOT_CREATE_IGNORE_FILE: 34
    readonly property int k_SYNC_CONFIG_READ_FAILURE: 35
    readonly property int k_UNKNOWN_DRIVE_PATH: 36
    readonly property int k_NOTIFICATION_SYSTEM_UNAVAILABLE: 38
    readonly property int k_UNABLE_TO_ADD_WATCH: 39
    readonly property int k_UNABLE_TO_OPEN_DATABASE: 41
    readonly property int k_INSUFFICIENT_DISK_SPACE: 42
    readonly property int k_FAILURE_ACCESSING_PERSISTENT_STORAGE: 43
    readonly property int k_MISMATCH_OF_ROOT_FSID: 44

    anchors.fill: parent
    implicitHeight: errorInfo.implicitHeight
    implicitWidth: errorInfo.implicitWidth

    function resetActionButtonsVisibility() {
        actionRetry.visible = false;
        actionGetMoreStorage.visible = false;
        actionRemoveSyncedFolder.visible = false;
        actionEnableSync.visible = false;
        actionRestoreSyncedFolder.visible = false;
    }

    onErrorIdChanged: {
        resetActionButtonsVisibility();
    }

    /*
      every error code have a state, with a list of visible action buttons.
    */
    states: [
        State {
            when: (errorId == k_LOCAL_PATH_TEMPORARY_UNAVAILABLE || errorId == k_LOCAL_PATH_UNAVAILABLE ||
                   errorId == k_COULD_NOT_CREATE_IGNORE_FILE || errorId == k_NOTIFICATION_SYSTEM_UNAVAILABLE ||
                   errorId == k_UNABLE_TO_ADD_WATCH || errorId == k_INSUFFICIENT_DISK_SPACE ||
                   errorId == k_FAILURE_ACCESSING_PERSISTENT_STORAGE || errorId == k_MISMATCH_OF_ROOT_FSID ||
                   errorId == k_SYNC_CONFIG_WRITE_FAILURE || errorId == k_SYNC_CONFIG_READ_FAILURE ||
                   errorId == k_UNABLE_TO_OPEN_DATABASE)

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
            when: errorId == k_REMOTE_NODE_INSIDE_RUBBISH || errorId == k_REMOTE_NODE_MOVED_TO_RUBBISH

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
                checkable: true

                Timer {
                    id: timerActionRetry

                    interval: root.timeToResetActionButtonState
                    repeat: false

                    onTriggered: {
                        actionRetry.checked = false;
                    }
                }

                onClicked: {
                    if (!timerActionRetry.running) {
                        syncSettings.resumeSync(index);
                        actionRetry.checked = true;
                        timerActionRetry.start();
                    }
                }
            }

            PrimaryButton {
                id: actionGetMoreStorage

                visible: false
                sizes: SmallSizes {}
                text: SettingsStrings.solveIssueGetMoreStorage
                checkable: true

                Timer {
                    id: timerActionGetMoreStorage

                    interval: root.timeToResetActionButtonState
                    repeat: false

                    onTriggered: {
                        actionGetMoreStorage.checked = false;
                    }
                }

                onClicked: {
                    if (!timerActionGetMoreStorage.running) {
                        syncSettings.openOverQuotaDialog();
                        actionGetMoreStorage.checked = true;
                        timerActionGetMoreStorage.start();
                    }
                }
            }

            PrimaryButton {
                id: actionRemoveSyncedFolder

                visible: false
                colors.background: ColorTheme.buttonError
                colors.pressed: ColorTheme.buttonErrorPressed
                colors.hover: ColorTheme.buttonErrorHover
                sizes: SmallSizes {}
                text: SettingsStrings.solveIssueRemoveSyncedFolder
                colors.text: ColorTheme.textOnColor
                icons.source: Images.trash_small_thin_outline
                icons.position: Icon.Position.LEFT
                icons.colorEnabled: ColorTheme.textOnColor
                checkable: true

                Timer {
                    id: timerActionRemoveSyncedFolder

                    interval: root.timeToResetActionButtonState
                    repeat: false

                    onTriggered: {
                        actionRemoveSyncedFolder.checked = false;
                    }
                }

                onClicked: {
                    if (!timerActionRemoveSyncedFolder.running) {
                        syncSettings.remove(index);
                        actionRemoveSyncedFolder.checked = true;
                        timerActionRemoveSyncedFolder.start();
                    }
                }
            }

            PrimaryButton {
                id: actionEnableSync

                visible: false
                sizes: SmallSizes {}
                text: SettingsStrings.solveIssueEnableSync
                icons.source: Images.power_small_thin_outline
                icons.position: Icon.Position.LEFT
                checkable: true

                Timer {
                    id: timerActionEnableSync

                    interval: root.timeToResetActionButtonState
                    repeat: false

                    onTriggered: {
                        actionEnableSync.checked = false;
                    }
                }

                onClicked: {
                    if (!timerActionEnableSync.running) {
                        syncSettings.resumeSync(index);
                        actionEnableSync.checked = true;
                        timerActionEnableSync.start();
                    }
                }
            }

            PrimaryButton {
                id: actionRestoreSyncedFolder

                visible: false
                sizes: SmallSizes {}
                text: SettingsStrings.solveIssueRestoreFolder
                icons.source: Images.trash_off_small_thin_outline
                icons.position: Icon.Position.LEFT
                checkable: true

                Timer {
                    id: timerActionRestoreSyncedFolder

                    interval: root.timeToResetActionButtonState
                    repeat: false

                    onTriggered: {
                        actionRestoreSyncedFolder.checked = false;
                    }
                }

                onClicked: {
                    if (!timerActionRestoreSyncedFolder.running) {
                        syncSettings.restoreSyncedFolder(index);
                        actionRestoreSyncedFolder.checked = true;
                        timerActionRestoreSyncedFolder.start();
                    }
                }
            }

            PrimaryButton {
                id: actionStartNewSync

                visible: false
                sizes: SmallSizes {}
                text: SettingsStrings.solveIssueStartNewSync
                icons.source: Images.sync_plus_small_thin_outline
                icons.position: Icon.Position.LEFT
                checkable: true

                Timer {
                    id: timerActionStartNewSync

                    interval: root.timeToResetActionButtonState
                    repeat: false

                    onTriggered: {
                        actionStartNewSync.checked = false;
                    }
                }

                onClicked: {
                    if (!timerActionStartNewSync.running) {
                        syncSettings.addSync();
                        actionStartNewSync.checked = true;
                        timerActionStartNewSync.start();
                    }
                }

            }
        }
    }
}