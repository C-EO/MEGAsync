import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Shapes 1.15

import common 1.0

Switch {
    id: root

    indicator: Rectangle {
        implicitWidth: 40
        implicitHeight: 20
        x: root.leftPadding
        y: parent.height / 2 - height / 2
        radius: 10
        color: root.checked ? ColorTheme.buttonPrimary : ColorTheme.surface1
        border.color: ColorTheme.buttonPrimary
        border.width: 2
        readonly property int borderSpacing: 2

        Rectangle {
            x: root.checked ? parent.width - width - parent.border.width : parent.border.width + parent.borderSpacing
            y: root.checked ? parent.border.width : parent.border.width + parent.borderSpacing
            width: root.checked ? parent.height - (parent.border.width * 2) : parent.height - (parent.border.width * 2) - (parent.borderSpacing * 2)
            height: width
            radius: height / 2.0
            color: root.checked ? ColorTheme.surface1 : ColorTheme.buttonPrimary
            border.color: color

            /*
            Shape {
                anchors.fill: parent

                ShapePath {
                    strokeWidth: 2
                    strokeColor: "red"
                    fillColor: "transparent"

                    capStyle: ShapePath.RoundCap
                    joinStyle: ShapePath.RoundJoin

                    startX: 4
                    startY: 5

                    PathLine { x: 6; y: 8 }
                    //PathLine { x: 45; y: 15 }
                }
            }
            */
        }
    }
}
