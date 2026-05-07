import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.images 1.0
import components.texts 1.0
import components.buttons 1.0

import SyncSettingsModel 1.0

Item {
    id: root

    readonly property int defaultTopMargin: 19
    readonly property int syncTableSpacing: 4
    readonly property int titleTextPixelSize: 10
    readonly property int syncTablePadding: 12
    readonly property int syncNameLabelWidth: 412
    readonly property int listViewOfSyncItems: 270
    readonly property int syncItemBackgroundRadius: 6
    readonly property int syncItemBackgroundHeight: 32
    readonly property int syncItemHoritzontalPadding: 12
    readonly property int syncItemContentSpacing: 4
    readonly property int syncItemContentNameWidth: 404
    readonly property int folderSearchIconSize: 16
    readonly property int verticalAddSyncButtonSeparator: 16
    readonly property int leftPaddingAddSyncButton: 12
    readonly property int rightPaddingAddSyncButton: 16

    Column {
        id: content

        anchors.fill: parent
        anchors.topMargin: defaultTopMargin
        width: parent.width
        spacing: syncTableSpacing

        Row {
            id: syncsColumnsLabels

            width: parent.width
            rightPadding: syncTablePadding
            leftPadding: rightPadding

            Text {
                id: syncName

                text: SettingsStrings.tableSyncsNameColumn
                font.pixelSize: root.titleTextPixelSize
                font.weight: Font.DemiBold
                elide: Text.ElideRight
                width: syncNameLabelWidth
            }

            Text {
                id: syncStatus

                text: SettingsStrings.tableSyncsStatusColumn
                font.pixelSize: root.titleTextPixelSize
                font.weight: Font.DemiBold
                elide: Text.ElideRight
            }
        }

        Rectangle {
            id: syncsUnderLineTitle

            width: parent.width
            height: Constants.dividerThickness
            color: ColorTheme.borderStrong
        }

        ListView {
            id: syncList

            height: listViewOfSyncItems
            width: parent.width
            model: syncSettingsModel
            delegate: syncComponent
            spacing: syncTableSpacing
        }

        Rectangle {
            id: syncsUnderLineTable

            width: parent.width
            height: Constants.dividerThickness
            color: ColorTheme.borderStrong
        }

        Item {
            height: verticalAddSyncButtonSeparator
            width: parent.width
        }

        Row {
            width: parent.width
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
                    syncSettings.addSync();
                }
            }
        }

        Item {
            height: 145
            width: parent.width
        }

        RowLayout {
            width: parent.width
            height: 58
            spacing: 0

            ColumnLayout {
                spacing: 4

                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.leftMargin: syncItemHoritzontalPadding

                Text {
                    id: title

                    text: qsTr("Automatic sync issue resolution")
                    font.pixelSize: 12
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                RichText {
                    id: description

                    manageMouse: true
                    manageHover: true
                    underlineLink: true
                    rawText: qsTr("MEGA automatically detects and resolves sync issues for you. Turn it off if you prefer to review and handle them manually. [A]Learn more[/A]")
                    font.pixelSize: 12
                    font.weight: Font.Normal
                    elide: Text.ElideRight
                    width: parent.width
                    url: serviceUrlsAccess.getCreateSyncHelpUrl()
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
                        Layout.minimumWidth: 12
                    }

                    Rectangle {
                        id: swith
                        Layout.preferredWidth: 40
                        Layout.preferredHeight: 20
                        color: "red"
                    }
                }

                Item {
                    Layout.fillHeight: true
                }
            }
        }
    }

    Component {
        id: syncComponent

        Rectangle {
            id: syncItem

            width: syncList.width
            height: syncItemBackgroundHeight
            color: syncItemContainsMouse ? ColorTheme.surface1 : ColorTheme.pageBackground
            radius: syncItemBackgroundRadius

            property bool syncItemContainsMouse : syncItemMouseArea.containsMouse || folderSearchMouseArea.containsMouse || menuIconMouseArea.containsMouse

            MouseArea {
                id: syncItemMouseArea

                anchors.fill: parent
                hoverEnabled: true
                propagateComposedEvents: true
            }

            RowLayout {
                id: syncRow

                anchors.fill: parent
                anchors.leftMargin: syncItemHoritzontalPadding
                anchors.rightMargin: syncItemHoritzontalPadding
                spacing: syncItemContentSpacing

                Row {
                    id: syncNameContent

                    Layout.preferredHeight: parent.height
                    Layout.preferredWidth: syncItemContentNameWidth
                    width: syncItemContentNameWidth
                    spacing: syncItemContentSpacing

                    Text {
                        id: syncName

                        text: name
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    SvgImage {
                        id: folderSearchIcon

                        color: ColorTheme.iconPrimary
                        source: Images.folder_search_small_thin_outline
                        sourceSize: Qt.size(folderSearchIconSize, folderSearchIconSize)
                        visible: syncItemContainsMouse
                        anchors.verticalCenter: parent.verticalCenter

                        MouseArea {
                            id: folderSearchMouseArea

                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor

                            onClicked: {
                                syncSettings.exploreLocalSync(folder);
                            }
                        }
                    }
                }

                RowLayout {
                    id: syncSatusContent

                    spacing: syncItemContentSpacing
                    Layout.fillWidth: true

                    SvgImage {
                        id: folderStatusIcon

                        color: ColorTheme.iconPrimary
                        source: Images.sync_01_small_thin_outline
                        sourceSize: Qt.size(folderSearchIconSize, folderSearchIconSize)
                    }

                    Text {
                        id: syncStatus

                        text: status
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    SvgImage {
                        id: menuIcon

                        color: ColorTheme.iconPrimary
                        source: Images.threeDots
                        sourceSize: Qt.size(folderSearchIconSize, folderSearchIconSize)
                        visible: syncItemContainsMouse

                        MouseArea {
                            id: menuIconMouseArea

                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                // open menu
                            }
                        }
                    }
                }
            }
        }
    }
}
