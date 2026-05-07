import QtQuick 2.15
import QtQuick.Controls 2.15

import common 1.0

Switch {
    id: root

    text: qsTr("Switch")

    indicator: Rectangle {
        implicitWidth: 48
        implicitHeight: 26
        x: root.leftPadding
        y: parent.height / 2 - height / 2
        radius: 13
        color: root.checked ? "#17a81a" : "#ffffff"
        border.color: root.checked ? "#17a81a" : "#cccccc"

        Rectangle {
            x: root.checked ? parent.width - width : 0
            width: 26
            height: 26
            radius: 13
            color: root.down ? "#cccccc" : "#ffffff"
            border.color: root.checked ? (root.down ? "#17a81a" : "#21be2b") : "#999999"
        }
    }

    contentItem: Text {
        text: root.text
        font: root.font
        opacity: enabled ? 1.0 : 0.3
        color: root.down ? "#17a81a" : "#21be2b"
        verticalAlignment: Text.AlignVCenter
        leftPadding: root.indicator.width + root.spacing
    }
}
