import QtQuick 2.15
import QtQuick.Controls 2.15 as Qml
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

import common 1.0

import components.texts 1.0 as Texts
import components.busyIndicator 1.0
import components.images 1.0

Qml.RoundButton {
    id: root

    property alias progressValue: backgroundProgress.value
    property alias underlineText: buttonText.font.underline
    property alias leftIconRotation: leftIconRotationItem

    property Colors colors: Colors {}
    property Icon icons: Icon {}
    property Sizes sizes: Sizes {}

    property int buttonCursorShape: Qt.PointingHandCursor

    function getBorderColor() {
        if(!root.enabled && !icons.busyIndicatorVisible) {
            return colors.borderDisabled;
        }
        if(root.pressed || root.checked) {
            return colors.borderPressed;
        }
        if(root.hovered) {
            return colors.borderHover;
        }
        return colors.border;
    }

    function getBackgroundColor() {
        if(!root.enabled && !icons.busyIndicatorVisible) {
            return colors.disabled;
        }
        if(root.pressed || root.checked) {
            return colors.pressed;
        }
        if(root.hovered) {
            return colors.hover;
        }
        return colors.background;
    }

    function getTextColor() {
        if(!root.enabled && !icons.busyIndicatorVisible) {
            return colors.textDisabled;
        }
        if(root.pressed || root.checked) {
            return colors.textPressed;
        }
        if(root.hovered) {
            return colors.textHover;
        }
        return colors.text;
    }

    function getIconColor() {
        if(!root.enabled && !icons.busyIndicatorVisible) {
            return icons.colorDisabled;
        }
        if(root.pressed || root.checked) {
            return icons.colorPressed;
        }
        if(root.hovered) {
            return icons.colorHovered;
        }
        return icons.colorEnabled;
    }

    bottomPadding: sizes.verticalPadding + sizes.focusBorderWidth
    topPadding: sizes.verticalPadding + sizes.focusBorderWidth
    leftPadding: sizes.horizontalPadding + sizes.focusBorderWidth
    rightPadding: sizes.horizontalPadding + sizes.focusBorderWidth
    height: 2 * sizes.verticalPadding + 2 * sizes.focusBorderWidth + contentRow.implicitHeight
    width: 2 * sizes.horizontalPadding + 2 * sizes.focusBorderWidth + contentRow.implicitWidth
    Layout.preferredHeight: height
    Layout.preferredWidth: width

    contentItem: Row {
        id: contentRow

        spacing: sizes.spacing

        SvgImage {
            id: leftImage

            anchors.verticalCenter: parent.verticalCenter
            source: root.icons.source
            color: getIconColor()
            sourceSize: sizes.iconSize
            visible: root.icons.source !== "" && !root.icons.busyIndicatorVisible
                        && (root.icons.position === Icon.Position.LEFT
                            || root.icons.position === Icon.Position.BOTH)

            RotationAnimation on rotation {
                id: leftIconRotationItem

                from: 0
                to: 360
                duration: 1000
                running: false
                loops: 5
            }
        }

        BusyIndicator {
            id: leftBusyIndicator

            anchors.verticalCenter: parent.verticalCenter
            color: root.icons.colorEnabled
            imageSize: sizes.iconSize
            visible: root.icons.busyIndicatorVisible
                        && root.icons.position === Icon.Position.LEFT
        }

        Texts.Text {
            id: buttonText

            // Make the text fill the full available width if no images or indicators are visible.
            width: sizes.fillWidth ? parent.width : implicitWidth
            anchors.verticalCenter: parent.verticalCenter
            text: root.text
            color: getTextColor()
            lineHeight: sizes.textLineHeight
            lineHeightMode: Text.FixedHeight
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            font {
                pixelSize: sizes.textFontSize
                weight: Font.DemiBold
            }
        }

        SvgImage {
            id: rightImage

            anchors.verticalCenter: parent.verticalCenter
            source: root.icons.source
            color: getIconColor()
            sourceSize: sizes.iconSize
            visible: root.icons.source !== "" && !root.icons.busyIndicatorVisible
                        && (root.icons.position === Icon.Position.RIGHT
                            || root.icons.position === Icon.Position.BOTH)
        }

        BusyIndicator {
            id: rightBusyIndicator

            anchors.verticalCenter: parent.verticalCenter
            color: icons.colorEnabled
            imageSize: sizes.iconSize
            visible: root.icons.busyIndicatorVisible
                        && root.icons.position === Icon.Position.RIGHT
        }
    }

    background: Rectangle {
        id: focusRect

        color: "transparent"
        border.color: root.enabled
                      ? (root.activeFocus ? ColorTheme.focusColor : "transparent")
                      : "transparent"
        border.width: sizes.focusBorderWidth
        radius: sizes.focusBorderRadius
        width: root.width
        height: root.height

        Rectangle {
            id: backgroundRect

            anchors {
                fill: focusRect
                margins: sizes.focusBorderWidth
            }
            border {
                width: sizes.borderWidth
                color: getBorderColor()
            }
            radius: sizes.radius
            color: getBackgroundColor()
            layer {
                enabled: true
                effect: OpacityMask {
                    id: mask

                    maskSource: Item {
                        width: backgroundRect.width
                        height: backgroundRect.height

                        Rectangle {
                            id: maskRect

                            anchors.centerIn: parent
                            width:  backgroundRect.width
                            height: backgroundRect.height
                            radius: sizes.radius
                        }
                    }
                }
            }

            Progress {
                id: backgroundProgress

                anchors.fill: parent
            }

        } // Rectangle: backgroundRect

    } // Rectangle: focusRect

    Keys.onReturnPressed: {
        root.clicked();
        event.accepted = false;
    }

    Keys.onEnterPressed: {
        root.clicked();
        event.accepted = false;
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        onPressed: mouse => { mouse.accepted = false; }
        cursorShape: buttonCursorShape
    }

}
