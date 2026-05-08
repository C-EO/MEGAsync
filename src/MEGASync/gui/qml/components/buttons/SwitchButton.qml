import QtQuick 2.15
import QtQuick.Controls 2.15

import common 1.0
import components.images 1.0

Switch {
    id: root

    property alias implicitHeight: background.implicitHeight
    property alias implicitWidth: background.implicitWidth

    readonly property int checkImageSize: 16
    readonly property int tickImageMarginsToHandle: 3
    readonly property int borderSpacing: 2
    readonly property int defaultHeight: 20
    readonly property int defaultWidth: 40
    readonly property int borderWidth: 2

    indicator: Rectangle {
        id: background

        implicitWidth: defaultWidth
        implicitHeight: defaultHeight
        x: root.leftPadding
        y: parent.height / 2 - height / 2
        radius: height / 2
        color: root.checked ? ColorTheme.buttonPrimary : ColorTheme.surface1
        border.color: ColorTheme.buttonPrimary
        border.width: borderWidth

        Rectangle {
            id: handle

            x: root.checked ? parent.width - width - parent.border.width : parent.border.width + borderSpacing
            y: root.checked ? parent.border.width : parent.border.width + borderSpacing
            width: root.checked ? parent.height - parent.border.width * 2 : parent.height - parent.border.width * 2 - borderSpacing * 2
            height: width
            radius: height / 2.0
            color: root.checked ? ColorTheme.surface1 : ColorTheme.buttonPrimary
            border.color: color

            SvgImage {
                id: tickImage

                anchors.fill: parent
                anchors.margins: tickImageMarginsToHandle
                color: ColorTheme.iconPrimary
                source: Images.check_medium_regular_solid
                sourceSize: Qt.size(check_image_size, check_image_size)
                visible: root.checked
            }
        }
    }
}
