import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

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
                font.weight: Font.Demibold
                elide: Text.ElideRight
                width: syncNameLabelWidth
            }

            Text {
                id: syncStatus

                text: SettingsStrings.tableSyncsStatusColumn
                font.pixelSize: root.titleTextPixelSize
                font.weight: Font.Demibold
                elide: Text.ElideRight
            }
        }

        Rectangle {
            id: storageDivider

            width: parent.width
            height: Constants.dividerThickness
            color: ColorTheme.borderStrong
        }

        ListView {
            id: syncList

            height: listViewOfSyncItems
            width: parent.width
            model: syncSettingsModel
            delegate: syncItem
        }
    }

    Component {
        id: syncItem

        Rectangle
        {
            width: syncList.width
            height: syncItemBackgroundHeight
            color: mouseArea.containsMouse ? ColorTheme.surface1 : ColorTheme.pageBackground
            radius: syncItemBackgroundRadius

            Row {
                id: syncRow

                anchors.fill: parent
                topPadding: syncItemVerticalPadding
                bottomPadding: topPadding
                leftPadding: syncItemHoritzontalPadding
                rightPadding: leftPadding
                spacing: syncItemContentSpacing

                Text {
                    id: syncName

                    text: folder
                    width: syncItemContentNameWidth
                }

                Text {
                    id: syncStatus

                    text: status
                }
            }

            MouseArea {
                id: mouseArea

                anchors.fill: parent
                hoverEnabled: true
            }
        }
    }
}
