import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.images 1.0
import components.texts 1.0
import components.buttons 1.0
import components.switch 1.0

import SyncSettingsModel 1.0

Item {
    id: root

    readonly property int defaultTopMargin: 19
    readonly property int syncTableSpacing: 4
    readonly property int syncStatusLabelWidth: 172
    readonly property int listViewOfSyncItems: 270
    readonly property int syncItemContentStatusWidth: 172
    readonly property int syncItemHoritzontalPadding: 12
    readonly property int verticalAddSyncButtonSeparator: 16
    readonly property int leftPaddingAddSyncButton: 12
    readonly property int rightPaddingAddSyncButton: 16
    readonly property int issueLabelPixelSize: 12
    readonly property int issuePartHeigh: 58
    readonly property int issuePartSpacing: 4
    readonly property int switchSeparator: 12

    function getStatusDescription(status) {
        switch(status) {
            case SyncSettingsModel.PENDING:
            case SyncSettingsModel.LOADING:
                return SettingsStrings.syncStateLoading;
            case SyncSettingsModel.SUSPENDED:
                return SettingsStrings.syncStatePaused;
            case SyncSettingsModel.FAIL:
                return SettingsStrings.syncStateDisabled;
            case SyncSettingsModel.SCANNING:
                return SettingsStrings.syncStateScanning;
            case SyncSettingsModel.ACTIVE:
                return SettingsStrings.syncStateSyncing;
            case SyncSettingsModel.IDLE:
                return SettingsStrings.syncStateSynced;
        }
    }

    SettingsEmptyState {
        visible: syncList.count === 0
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        image: Images.megaDevices
        title: SettingsStrings.titleNoSync
        description: SettingsStrings.descriptionNoSync
        buttonText: SettingsStrings.addSync
        onAddClicked: syncSettingsAccess.addItem()
    }

    ColumnLayout {
        id: content

        anchors.fill: parent
        anchors.topMargin: defaultTopMargin
        spacing: syncTableSpacing
        visible: syncList.count > 0

        SettingsSortableHeader {
            Layout.preferredWidth: parent.width
            nameText: SettingsStrings.tableSyncsNameColumn
            statusText: SettingsStrings.tableSyncsStatusColumn
            statusLabelWidth: root.syncStatusLabelWidth
            onSortByName: syncSettingsAccess.sortModelByName(ascending)
            onSortByStatus: syncSettingsAccess.sortModelByStatus(ascending)
        }

        Rectangle {
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: Constants.dividerThickness
            color: ColorTheme.borderStrong
        }

        ListView {
            id: syncList

            Layout.preferredWidth: parent.width
            Layout.minimumHeight: listViewOfSyncItems
            Layout.fillHeight: true
            Layout.maximumHeight: Math.max(listViewOfSyncItems, contentHeight)

            model: syncSettingsModel
            spacing: syncTableSpacing
            interactive: contentHeight > height
            clip: true

            delegate: SettingsListDelegate {
                settingsAccess: syncSettingsAccess
                statusContentWidth: root.syncItemContentStatusWidth
                statusDescription: root.getStatusDescription
            }

            ScrollBar.vertical: ScrollBar {
                policy: syncList.contentHeight > syncList.height
                      ? ScrollBar.AlwaysOn
                      : ScrollBar.AlwaysOff
            }
        }

        Rectangle {
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: Constants.dividerThickness
            color: ColorTheme.borderStrong
        }

        Item {
            Layout.preferredHeight: verticalAddSyncButtonSeparator
            Layout.preferredWidth: parent.width
        }

        Row {
            Layout.preferredWidth: parent.width
            layoutDirection: Qt.RightToLeft

            PrimaryButton {
                id: addSyncButton

                text: SettingsStrings.addSync
                icons.source: Images.plus
                icons.position: Icon.Position.LEFT
                leftPadding: leftPaddingAddSyncButton
                rightPadding: rightPaddingAddSyncButton
                width: implicitWidth
                onClicked: {
                    syncSettingsAccess.addItem();
                }
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width
        }

        RowLayout {
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: issuePartHeigh
            Layout.maximumHeight: issuePartHeigh
            spacing: 0

            ColumnLayout {
                spacing: issuePartSpacing

                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.leftMargin: syncItemHoritzontalPadding

                Text {
                    id: title

                    text: SettingsStrings.syncIssueTitle
                    font.pixelSize: issueLabelPixelSize
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                RichText {
                    id: description

                    manageMouse: true
                    manageHover: true
                    underlineLink: true
                    rawText: SettingsStrings.syncIssueDescription
                    font.pixelSize: issueLabelPixelSize
                    font.weight: Font.Normal
                    elide: Text.ElideRight
                    width: parent.width
                    url: serviceUrlsAccess.getSyncHelpUrl()
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }

            ColumnLayout {
                spacing: 0

                RowLayout {
                    spacing: 0

                    Item {
                        Layout.fillWidth: true
                        Layout.minimumWidth: switchSeparator
                    }

                    Switch {
                        id: fixIssue

                        checked: syncSettingsAccess.automaticSyncIssueResolverEnabled
                        onCheckedChanged: {
                            syncSettingsAccess.automaticSyncIssueResolverEnabled = checked;
                        }
                    }
                }

                Item {
                    Layout.fillHeight: true
                }
            }
        }
    }
}
