import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.images 1.0
import components.texts 1.0

import SyncSettingsModel 1.0

Item {
    id: root

    readonly property int defaultTopMargin: 19
    readonly property int tableTitleUnderLineSpacing: 4
    readonly property int titleTextPixelSize: 10
    readonly property int syncTablePadding: 12
    readonly property int syncNameLabelWidth: 412
    readonly property int listViewOfSyncItems: 270
    readonly property int syncItemBackgroundRadius: 6
    readonly property int syncItemBackgroundHeight: 32
    readonly property int syncItemVerticalPadding: 7
    readonly property int syncItemHoritzontalPadding: 12
    readonly property int syncItemContentSpacing: 4
    readonly property int syncItemContentNameWidth: 404
    readonly property int folderSearchIconSize: 16

    Column {
        id: content

        anchors.fill: parent
        anchors.topMargin: defaultTopMargin
        width: parent.width
        spacing: tableTitleUnderLineSpacing

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
        }
    }

    Component {
        id: syncComponent

        Rectangle {
            id: syncItem

            width: syncList.width
            height: syncItemBackgroundHeight
            color: syncItemMouseArea.containsMouse ? ColorTheme.surface1 : ColorTheme.pageBackground
            radius: syncItemBackgroundRadius

            MouseArea {
                id: syncItemMouseArea

                anchors.fill: parent
                hoverEnabled: true
                propagateComposedEvents: true
                z: 0
            }

            RowLayout {
                id: syncRow

                anchors.fill: parent
                anchors.topMargin: syncItemVerticalPadding
                anchors.bottomMargin: syncItemVerticalPadding
                anchors.leftMargin: syncItemHoritzontalPadding
                anchors.rightMargin: syncItemHoritzontalPadding
                spacing: syncItemContentSpacing

                Row {
                    id: syncNameContent

                    Layout.preferredWidth: syncItemContentNameWidth
                    width: syncItemContentNameWidth
                    spacing: syncItemContentSpacing

                    Text {
                        id: syncName

                        text: folder
                    }

                    SvgImage {
                        id: folderSearchIcon

                        color: ColorTheme.iconPrimary
                        source: Images.folder_search_small_thin_outline
                        sourceSize: Qt.size(folderSearchIconSize, folderSearchIconSize)
                        visible: syncItemMouseArea.containsMouse

                        MouseArea {
                            id: folderSearchMouseArea

                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor

                            onClicked: {
                                // open folder
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
                        visible: syncItemMouseArea.containsMouse

                        MouseArea {
                            id: menuIconMouseArea

                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor

                            onClicked: {
                                // open folder
                            }
                        }
                    }
                }
            }
        }
    }
}
