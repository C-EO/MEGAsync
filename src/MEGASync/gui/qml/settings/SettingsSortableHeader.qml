import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.images 1.0

/*
  Shared sortable column header (Name + Status) for the Syncs and Backups settings
  tabs. Tracks ascending/descending per column and which order arrow is shown, and
  notifies the owner through sortByName/sortByStatus.
*/
RowLayout {
    id: root

    property string nameText
    property string statusText
    property color labelColor: ColorTheme.textPrimary
    property bool showInitialNameIndicator: true

    property int statusLabelWidth: 156
    property int tablePadding: 12
    property int titlePixelSize: 10
    property int contentSpacing: 4
    property size iconOrderFlagSize: Qt.size(12, 12)

    signal sortByName(bool ascending)
    signal sortByStatus(bool ascending)

    Layout.rightMargin: tablePadding
    Layout.leftMargin: tablePadding
    spacing: 0

    Item {
        id: nameHeaderArea

        Layout.fillWidth: true
        implicitHeight: nameColumnContent.implicitHeight
        Layout.alignment: Qt.AlignVCenter

        RowLayout {
            anchors.fill: parent
            spacing: 0

            RowLayout {
                id: nameColumnContent

                spacing: 0
                Layout.alignment: Qt.AlignVCenter

                Text {
                    id: nameColumn

                    property bool sortByNameAscending: true

                    text: root.nameText
                    color: root.labelColor
                    font.pixelSize: root.titlePixelSize
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                }

                Item {
                    Layout.preferredWidth: root.contentSpacing
                    Layout.maximumWidth: root.contentSpacing
                    Layout.minimumWidth: root.contentSpacing
                }

                SvgImage {
                    id: nameColumnOrderSymbol

                    source: nameColumn.sortByNameAscending ? Images.arrow_up_medium_regular_outline : Images.arrow_down_medium_regular_outline
                    visible: root.showInitialNameIndicator
                    sourceSize: root.iconOrderFlagSize
                    color: ColorTheme.iconPrimary
                }
            }

            Item {
                id: spacer
                Layout.fillWidth: true
            }
        }

        MouseArea {
            id: mouseAreaNameColumnSpace

            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor

            onClicked: {
                nameColumn.sortByNameAscending = !nameColumn.sortByNameAscending
                root.sortByName(nameColumn.sortByNameAscending)
                nameColumnOrderSymbol.visible = true
                statusColumnOrderSymbol.visible = false
            }
        }
    }

    Item {
        id: statusHeaderArea

        implicitWidth: statusColumnContent.implicitWidth
        implicitHeight: statusColumnContent.implicitHeight
        Layout.preferredHeight: parent.height
        Layout.preferredWidth: root.statusLabelWidth
        Layout.maximumWidth: root.statusLabelWidth
        Layout.alignment: Qt.AlignVCenter

        RowLayout {
            anchors.fill: parent
            spacing: 0

            RowLayout {
                id: statusColumnContent

                spacing: 0
                Layout.alignment: Qt.AlignVCenter

                Text {
                    id: statusColumn

                    property bool sortByStatusAscending: true

                    text: root.statusText
                    color: root.labelColor
                    font.pixelSize: root.titlePixelSize
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                }

                Item {
                    Layout.preferredWidth: root.contentSpacing
                    Layout.maximumWidth: root.contentSpacing
                }

                SvgImage {
                    id: statusColumnOrderSymbol

                    source: statusColumn.sortByStatusAscending ? Images.arrow_up_medium_regular_outline : Images.arrow_down_medium_regular_outline
                    visible: false
                    color: ColorTheme.iconPrimary
                    sourceSize: root.iconOrderFlagSize
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }

        MouseArea {
            id: mouseAreaStatusColumnSpace

            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                statusColumn.sortByStatusAscending = !statusColumn.sortByStatusAscending
                root.sortByStatus(statusColumn.sortByStatusAscending)
                nameColumnOrderSymbol.visible = false
                statusColumnOrderSymbol.visible = true
            }
        }
    }
}
