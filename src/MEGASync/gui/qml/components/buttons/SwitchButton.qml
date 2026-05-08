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

    function getBackgroundColor() {
        var color = ColorTheme.surface1

        if (root.checked) {
            if (root.pressed) {
                color = ColorTheme.buttonPrimaryPressed
            }
            else if (root.hovered) {
                color = ColorTheme.buttonPrimaryHover
            }
            else if (!root.enabled) {
                color = ColorTheme.buttonDisabled
            }
            else {
                color = ColorTheme.buttonPrimary
            }
        }

        return color;
    }

    function getBackgroundBorderColor() {
        if (root.pressed) {
            return ColorTheme.buttonPrimaryPressed
        }
        else if (root.hovered) {
            return ColorTheme.buttonPrimaryHover
        }
        else if (!root.enabled) {
            return ColorTheme.buttonDisabled
        }
        else {
            return ColorTheme.buttonPrimary;
        }
    }

    function getHandleColor() {
        var color = ColorTheme.surface1

        if (!root.checked) {
            if (root.pressed) {
                return ColorTheme.buttonPrimaryPressed
            }
            else if (root.hovered) {
                color = ColorTheme.buttonPrimaryHover
            }
            else if (!root.enabled) {
                return ColorTheme.buttonDisabled;
            }
            else {
                color = ColorTheme.buttonPrimary
            }
        }

        return color;
    }

    indicator: Rectangle {
        id: background

        implicitWidth: defaultWidth
        implicitHeight: defaultHeight
        x: root.leftPadding
        y: parent.height / 2 - height / 2
        radius: height / 2
        color: getBackgroundColor()
        border.color: getBackgroundBorderColor();
        border.width: borderWidth

        Rectangle {
            id: handle

            x: root.checked ? parent.width - width - parent.border.width : parent.border.width + borderSpacing
            y: root.checked ? parent.border.width : parent.border.width + borderSpacing
            width: root.checked ? parent.height - parent.border.width * 2 : parent.height - parent.border.width * 2 - borderSpacing * 2
            height: width
            radius: height / 2.0
            color: getHandleColor()
            border.color: color

            Behavior on x {
                NumberAnimation {
                    duration: 100
                    easing.type: Easing.InOutQuad
                }
            }

            Behavior on y {
                NumberAnimation {
                    duration: 100
                    easing.type: Easing.InOutQuad
                }
            }

            Behavior on width {
                NumberAnimation {
                    duration: 100
                    easing.type: Easing.InOutQuad
                }
            }

            SvgImage {
                id: tickImage

                anchors.fill: parent
                anchors.margins: tickImageMarginsToHandle
                color: getBackgroundColor() // tick color is exactly the same as the background color.
                source: Images.check_medium_regular_solid
                sourceSize: Qt.size(root.check_image_size, root.check_image_size)
                visible: root.checked
            }
        }
    }
}
