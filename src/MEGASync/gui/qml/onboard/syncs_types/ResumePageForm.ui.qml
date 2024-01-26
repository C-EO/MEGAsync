import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0
import components.buttons 1.0
import components.pages 1.0

import backups 1.0

import onboard 1.0

FooterButtonsPage {
    id: root

    readonly property string stateFullSync: "FULL"
    readonly property string stateSelectiveSync: "SELECTIVE"
    readonly property string stateBackup: "BACKUP"
    readonly property int maxSizeDescription: 80
    readonly property int buttonQuestionMargin: 24

    property alias buttonGroup: buttonGroupItem
    property alias syncButton: syncButtonItem

    property string title
    property string description
    property bool fullSyncDone
    property bool selectiveSyncDone

    footerButtons {
        leftSecondary.visible: false
        rightSecondary.text: Strings.viewInSettings
        rightPrimary {
            text: Strings.done
            icons: Icon {}
        }
    }

    states: [
        State {
            name: stateFullSync
            PropertyChanges { target: titleItem; text: OnboardingStrings.finalStepSyncTitle; }
            PropertyChanges { target: descriptionItem; text: OnboardingStrings.finalStepSync; }
            PropertyChanges { target: syncButtonItem; visible: false; }
            PropertyChanges {
                target: stepPanel;
                state: stepPanel.stepAllDone;
                step3Text: OnboardingStrings.syncChooseType;
                step4Text: OnboardingStrings.syncSetUp;
            }
        },
        State {
            name: stateSelectiveSync
            extend: stateFullSync
            PropertyChanges {
                target: syncButtonItem;
                type: SyncsType.Types.SELECTIVE_SYNC;
                visible: true;
            }
        },
        State {
            name: stateBackup
            PropertyChanges { target: titleItem; text: BackupsStrings.finalStepBackupTitle; }
            PropertyChanges { target: descriptionItem; text: BackupsStrings.finalStepBackup; }
            PropertyChanges {
                target: syncButtonItem;
                type: !fullSyncDone && !selectiveSyncDone
                      ? SyncsType.Types.SYNC
                      : SyncsType.Types.SELECTIVE_SYNC;
                visible: !fullSyncDone;
                title: !fullSyncDone && !selectiveSyncDone
                       ? OnboardingStrings.sync
                       : OnboardingStrings.selectiveSync;
                description: !fullSyncDone && !selectiveSyncDone
                             ? OnboardingStrings.finalPageButtonSync
                             : OnboardingStrings.finalPageButtonSelectiveSync;
            }
            PropertyChanges {
                target: stepPanel;
                state: stepPanel.stepAllDone;
                step3Text: OnboardingStrings.backupSelectFolders;
                step4Text: OnboardingStrings.backupConfirm;
            }
        }
    ]

    ColumnLayout {
        id: mainLayout

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        Texts.Text {
            id: titleItem

            Layout.preferredWidth: parent.width
            text: title
            font {
                pixelSize: Texts.Text.Size.LARGE
                weight: Font.Bold
            }
            wrapMode: Text.Wrap
        }

        Texts.Text {
            id: descriptionItem

            Layout.preferredWidth: parent.width
            Layout.topMargin: 8
            text: description
            font.pixelSize: Texts.Text.Size.MEDIUM
            wrapMode: Text.Wrap
        }

        Texts.Text {
            id: finalStepQuestionText

            Layout.preferredWidth: parent.width
            Layout.topMargin: (descriptionItem.height > maxSizeDescription) ? (buttonQuestionMargin * 0.5) : buttonQuestionMargin
            text: OnboardingStrings.finalStepQuestion
            font {
                pixelSize: Texts.Text.Size.MEDIUM_LARGE
                weight: Font.DemiBold
            }
        }

        Item {
            id: buttons

            Layout.preferredWidth: parent.width + 8
            Layout.topMargin: (descriptionItem.height > maxSizeDescription) ? (buttonQuestionMargin * 0.5) : buttonQuestionMargin

            ButtonGroup {
                id: buttonGroupItem
            }

            RowLayout {
                id: buttonsLayout

                anchors {
                    fill: parent
                    leftMargin: -syncButtonItem.focusBorderWidth
                    rightMargin: backupsButton.focusBorderWidth
                }
                spacing: 12

                SyncsVerticalButton {
                    id: syncButtonItem

                    width: (parent.width - parent.spacing) / 2
                    Layout.preferredWidth: width
                    Layout.preferredHeight: height
                    contentMargin: 24
                    contentSpacing: 8
                    imageSourceSize: Qt.size(22, 20)
                    title: OnboardingStrings.selectiveSync
                    description: OnboardingStrings.finalPageButtonSelectiveSync
                    imageSource: Images.sync
                    checkable: false
                    focus: true
                    useMaxSiblingHeight: true
                    ButtonGroup.group: buttonGroupItem
                }

                SyncsVerticalButton {
                    id: backupsButton

                    width: !syncButtonItem.visible
                           ? parent.width
                           : (parent.width - parent.spacing) / 2
                    Layout.preferredWidth: width
                    Layout.preferredHeight: height
                    contentMargin: 24
                    contentSpacing: 8
                    imageSourceSize: Qt.size(22, 20)
                    title: OnboardingStrings.backup
                    description: OnboardingStrings.finalPageButtonBackup
                    imageSource: Images.installationTypeBackups
                    type: SyncsType.Types.BACKUP
                    checkable: false
                    useMaxSiblingHeight: syncButtonItem.visible
                    ButtonGroup.group: buttonGroupItem
                }

            } // RowLayout: buttonsLayout

        } // Item: buttons

    } // ColumnLayout: mainLayout

}
