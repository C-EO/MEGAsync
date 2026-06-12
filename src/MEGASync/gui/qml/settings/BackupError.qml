import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.images 1.0
import components.texts 1.0
import components.buttons 1.0
import components.menus 1.0

import SyncErrors 1.0

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
        actionStartNewBackup.visible = false;
    }

    onErrorIdChanged: {
        resetActionButtonsVisibility();
    }

    /*
      Every error code has a state with a list of visible action buttons.
    */
    states: [
        State {
            when: (errorId == SyncErrors.LOCAL_PATH_TEMPORARY_UNAVAILABLE ||
                   errorId == SyncErrors.NOTIFICATION_SYSTEM_UNAVAILABLE || errorId == SyncErrors.UNABLE_TO_ADD_WATCH ||
                   errorId == SyncErrors.INSUFFICIENT_DISK_SPACE || errorId == SyncErrors.FAILURE_ACCESSING_PERSISTENT_STORAGE ||
                   errorId == SyncErrors.MISMATCH_OF_ROOT_FSID || errorId == SyncErrors.SYNC_CONFIG_WRITE_FAILURE ||
                   errorId == SyncErrors.SYNC_CONFIG_READ_FAILURE || errorId == SyncErrors.UNABLE_TO_OPEN_DATABASE ||
                   errorId == SyncErrors.UNKNOWN_DRIVE_PATH || errorId == SyncErrors.LOCAL_PATH_UNAVAILABLE)

            PropertyChanges {
                target: actionRetry
                visible: true
            }
        },
        State {
            when: errorId == SyncErrors.STORAGE_OVERQUOTA

            PropertyChanges {
                target: actionGetMoreStorage
                visible: true
            }
        },
        State {
            when: errorId == SyncErrors.LOCAL_FILESYSTEM_MISMATCH

            PropertyChanges {
                target: actionStartNewBackup
                visible: true
            }
        },
        State {
            when: errorId == SyncErrors.LOGGED_OUT

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
            id: actionStartNewBackup

            visible: false
            sizes: SmallSizes {
                verticalPadding: root.actionButtonVerticalPadding
            }
            text: SettingsStrings.solveIssueStartNewBackup
            icons.source: Images.database_plus_small_thin_outline
            icons.position: Icon.Position.LEFT
            checkable: true

            Timer {
                id: timerActionStartNewBackup

                interval: root.timeToResetActionButtonState
                repeat: false

                onRunningChanged: {
                    if (running) {
                        settingsAccess.addItem();

                        actionStartNewBackup.checked = true;
                        actionStartNewBackup.buttonCursorShape = Qt.ArrowCursor
                        actionStartNewBackup.leftIconRotation.duration = root.buttonIconbusyAnimationDurationTime
                        actionStartNewBackup.leftIconRotation.loops = root.timeToResetActionButtonState / actionRetry.leftIconRotation.duration
                        actionStartNewBackup.leftIconRotation.start();
                    }
                    else {
                        actionStartNewBackup.checked = false;
                        actionStartNewBackup.buttonCursorShape = Qt.PointingHandCursor
                    }
                }
            }

            onClicked: {
                if (!timerActionStartNewBackup.running) {
                    timerActionStartNewBackup.start();
                }
            }
        }

        PrimaryButton {
            id: actionRemoveBackup

            visible: true
            colors.background: ColorTheme.buttonError
            colors.pressed: ColorTheme.buttonErrorPressed
            colors.hover: ColorTheme.buttonErrorHover
            sizes: SmallSizes {}
            text: SettingsStrings.menuActionsStopBackup
            colors.text: ColorTheme.textOnColor
            colors.textHover: ColorTheme.textOnColor
            colors.textPressed: ColorTheme.textOnColor
            icons.source: Images.database_x_medium_thin_outline
            icons.position: Icon.Position.LEFT
            icons.colorEnabled: ColorTheme.textOnColor
            icons.colorHovered: ColorTheme.textOnColor
            icons.colorPressed: ColorTheme.textOnColor
            checkable: true

            Timer {
                id: timerActionRemoveBackup

                interval: root.timeToResetActionButtonState
                repeat: false

                onRunningChanged: {
                    if (running) {
                        settingsAccess.remove(itemIndex);

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
