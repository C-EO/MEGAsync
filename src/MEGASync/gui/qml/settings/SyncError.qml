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

    anchors.fill: parent
    implicitHeight: errorInfo.implicitHeight
    implicitWidth: errorInfo.implicitWidth

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

                visible: true
                sizes: SmallSizes {}
                text: SettingsStrings.solveIssueSetFolderPermissions

                onClicked: {
                    syncSettings.getMoreSpace();
                }
            }

            PrimaryButton {
                id: actionRemoveSyncedFolder

                visible: true
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

                visible: true
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

                visible: true
                sizes: SmallSizes {}
                text: SettingsStrings.solveIssueRestoreFolder
                icons.source: Images.trash_off_small_thin_outline
                icons.position: Icon.Position.LEFT
                onClicked: {
                    syncSettings.restoreSyncedFolder(index);
                }
            }
        }
    }
}