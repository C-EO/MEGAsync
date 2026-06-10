import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.images 1.0
import components.texts 1.0
import components.buttons 1.0
import components.menus 1.0

ColumnLayout {
    id: root

    property var settingsAccess
    property int errorId
    property string errorMessage
    property string localFolder
    property int itemIndex

    readonly property int issueHandlerSpacing: 4
    readonly property int topErrorLabelMargin: 8
    readonly property int errorLabelMargin: 10
    readonly property int issueLabelPixelSize: 12
    readonly property int buttonActionSpacing: 0
    readonly property int actionButtonVerticalPadding: 3
    readonly property int buttonFocusBorder: 4
    readonly property int timeToResetActionButtonState: 5000
    readonly property int buttonIconbusyAnimationDurationTime: 1000

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

    // Exposed as a plain property (not a binding) so external items can size
    // themselves from this value without binding to implicitHeight directly.
    // Binding to ColumnLayout.implicitHeight from outside the component causes
    // Qt 5.15's layout engine to emit implicitHeightChanged during its own
    // evaluation, which Qt detects as a binding loop.
    property real contentHeight: 0
    Component.onCompleted: contentHeight = implicitHeight
    onImplicitHeightChanged: contentHeight = implicitHeight

    width: parent ? parent.width : implicitWidth
    height: implicitHeight
    spacing: issueHandlerSpacing

    function resetActionButtonsVisibility() {
        actionRetry.visible = false;
        actionGetMoreStorage.visible = false;
        actionEnableBackup.visible = false;
    }

    onErrorIdChanged: {
        resetActionButtonsVisibility();
    }

    /*
      Every error code has a state with a list of visible action buttons.
    */
    states: [
        State {
            when: (errorId == k_LOCAL_PATH_TEMPORARY_UNAVAILABLE || errorId == k_COULD_NOT_CREATE_IGNORE_FILE ||
                   errorId == k_NOTIFICATION_SYSTEM_UNAVAILABLE || errorId == k_UNABLE_TO_ADD_WATCH ||
                   errorId == k_INSUFFICIENT_DISK_SPACE || errorId == k_FAILURE_ACCESSING_PERSISTENT_STORAGE ||
                   errorId == k_MISMATCH_OF_ROOT_FSID || errorId == k_SYNC_CONFIG_WRITE_FAILURE ||
                   errorId == k_SYNC_CONFIG_READ_FAILURE || errorId == k_UNABLE_TO_OPEN_DATABASE ||
                   errorId == k_UNKNOWN_DRIVE_PATH || errorId == k_LOCAL_PATH_UNAVAILABLE)

            PropertyChanges {
                target: actionRetry
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
            when: errorId == k_LOGGED_OUT

            PropertyChanges {
                target: actionEnableBackup
                visible: true
            }
        }
    ]

    Text {
        id: errorLabel

        Layout.topMargin: root.topErrorLabelMargin
        Layout.rightMargin: root.errorLabelMargin
        Layout.leftMargin: root.errorLabelMargin
        Layout.bottomMargin: root.issueHandlerSpacing
        Layout.fillWidth: true
        text: root.errorMessage
        font.pixelSize: root.issueLabelPixelSize
        font.weight: Font.Normal
        elide: Text.ElideRight
        color: ColorTheme.textPrimary
    }

    RowLayout {
        id: buttonList

        spacing: root.buttonActionSpacing
        Layout.bottomMargin: root.errorLabelMargin - buttonFocusBorder
        Layout.rightMargin: root.errorLabelMargin
        Layout.leftMargin: Layout.bottomMargin

        PrimaryButton {
            id: actionRetry

            visible: false
            sizes: SmallSizes {
                verticalPadding: root.actionButtonVerticalPadding
            }
            icons.source: Images.rotate_cw_small_thin_outline
            icons.position: Icon.Position.LEFT
            text: SettingsStrings.solveIssueButtonRetry
            checkable: true

            Timer {
                id: timerActionRetry

                interval: root.timeToResetActionButtonState
                repeat: false

                onRunningChanged: {
                    if (running) {
                        settingsAccess.resume(itemIndex);

                        actionRetry.checked = true;
                        actionRetry.buttonCursorShape = Qt.ArrowCursor
                        actionRetry.leftIconRotation.duration = root.buttonIconbusyAnimationDurationTime
                        actionRetry.leftIconRotation.loops = root.timeToResetActionButtonState / actionRetry.leftIconRotation.duration
                        actionRetry.leftIconRotation.start();
                    }
                    else {
                        actionRetry.checked = false;
                        actionRetry.buttonCursorShape = Qt.PointingHandCursor
                    }
                }
            }

            onClicked: {
                if (!timerActionRetry.running) {
                    timerActionRetry.start();
                }
            }
        }

        PrimaryButton {
            id: actionGetMoreStorage

            visible: false
            sizes: SmallSizes {
                verticalPadding: root.actionButtonVerticalPadding
            }
            text: SettingsStrings.solveIssueGetMoreStorage
            checkable: true

            Timer {
                id: timerActionGetMoreStorage

                interval: root.timeToResetActionButtonState
                repeat: false

                onRunningChanged: {
                    if (running) {
                        settingsAccess.openOverQuotaDialog();

                        actionGetMoreStorage.checked = true;
                        actionGetMoreStorage.buttonCursorShape = Qt.ArrowCursor
                    }
                    else {
                        actionGetMoreStorage.checked = false;
                        actionGetMoreStorage.buttonCursorShape = Qt.PointingHandCursor
                    }
                }
            }

            onClicked: {
                if (!timerActionGetMoreStorage.running) {
                    timerActionGetMoreStorage.start();
                }
            }
        }

        PrimaryButton {
            id: actionEnableBackup

            visible: false
            sizes: SmallSizes {
                verticalPadding: root.actionButtonVerticalPadding
            }
            text: SettingsStrings.solveIssueEnableBackup
            icons.source: Images.power_small_thin_outline
            icons.position: Icon.Position.LEFT
            checkable: true

            Timer {
                id: timerActionEnableBackup

                interval: root.timeToResetActionButtonState
                repeat: false

                onRunningChanged: {
                    if (running) {
                        settingsAccess.resume(itemIndex);

                        actionEnableBackup.checked = true;
                        actionEnableBackup.buttonCursorShape = Qt.ArrowCursor
                        actionEnableBackup.leftIconRotation.duration = root.buttonIconbusyAnimationDurationTime
                        actionEnableBackup.leftIconRotation.loops = root.timeToResetActionButtonState / actionRetry.leftIconRotation.duration
                        actionEnableBackup.leftIconRotation.start();
                    }
                    else {
                        actionEnableBackup.checked = false;
                        actionEnableBackup.buttonCursorShape = Qt.PointingHandCursor
                    }
                }
            }

            onClicked: {
                if (!timerActionEnableBackup.running) {
                    timerActionEnableBackup.start();
                }
            }
        }

        PrimaryButton {
            id: actionRemoveBackup

            visible: true
            sizes: SmallSizes {
                verticalPadding: root.actionButtonVerticalPadding
            }
            text: SettingsStrings.menuActionsStopBackup
            icons.source: Images.database_x_medium_thin_outline
            icons.position: Icon.Position.LEFT
            checkable: true

            Timer {
                id: timerActionRemoveBackup

                interval: root.timeToResetActionButtonState
                repeat: false

                onRunningChanged: {
                    if (running) {
                        settingsAccess.removeNonConfirmation(itemIndex);

                        actionRemoveBackup.checked = true;
                        actionRemoveBackup.buttonCursorShape = Qt.ArrowCursor
                    }
                    else {
                        actionRemoveBackup.checked = false;
                        actionRemoveBackup.buttonCursorShape = Qt.PointingHandCursor
                    }
                }
            }

            onClicked: {
                if (!timerActionRemoveBackup.running) {
                    timerActionRemoveBackup.start();
                }
            }
        }
    }
}
