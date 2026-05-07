import QtQuick 2.15
import QtQuick.Controls 2.15

import common 1.0

Switch {
    id: root

    indicator: Rectangle {
        implicitWidth: 40
        implicitHeight: 20
        x: root.leftPadding
        y: parent.height / 2 - height / 2
        radius: 10
        color: root.checked ? ColorTheme.buttonPrimary : "transparent"
        border.color: root.checked ? ColorTheme.buttonPrimary : "#cccccc"
        border.width: 2
        readonly property int borderSpacing: 2

        Rectangle {
            x: root.checked ? parent.width - width - parent.border.width - parent.borderSpacing : parent.border.width + parent.borderSpacing
            y: parent.border.width + parent.borderSpacing
            width: parent.height - (parent.border.width * 2) - (parent.borderSpacing * 2)
            height: width
            radius: height / 2.0
            color: root.checked ? "red" : ColorTheme.buttonPrimary
            //border.color: root.checked ? (root.down ? "#17a81a" : "#21be2b") : "#999999"
        }
    }
}
