import QtQuick 2.15
import QtQuick.Layouts 1.15

import components.texts 1.0 as Texts

import MessageDialogTextInfo 1.0

Loader {
    id: root

    required property var textInfo

    required property real textLineHeight
    required property real textPixelSize
    required property real textWeight
    property real textLeftPadding : 0

    Layout.fillWidth: true
    Layout.preferredHeight: item ? item.implicitHeight : 0
    visible: root.textInfo ? root.textInfo.text !== "" : false
    active: root.textInfo !== null
    asynchronous: false

    sourceComponent: {
        if (!root.textInfo) {
            return null;
        }

        switch (root.textInfo.format) {
            case MessageDialogTextInfo.TextFormat.RICH:
                return richTextComponent;
            case MessageDialogTextInfo.TextFormat.PLAIN:
            default:
                return textComponent;
        }
    }

    Component {
        id: textComponent

        Texts.Text {
            width: root.width
            lineHeightMode: Text.FixedHeight
            lineHeight: root.textLineHeight
            wrapMode: Text.Wrap
            font {
                pixelSize: root.textPixelSize
                weight: root.textWeight
            }
            text: root.textInfo ? root.textInfo.text : ""
            leftPadding: textLeftPadding
        }
    }

    Component {
        id: richTextComponent

        Texts.RichText {
            width: root.width
            lineHeightMode: Text.FixedHeight
            lineHeight: root.textLineHeight
            wrapMode: Text.Wrap
            font {
                pixelSize: root.textPixelSize
                weight: root.textWeight
            }
            rawText: root.textInfo ? root.textInfo.text : ""

            Component.onCompleted: {
                if (hasLink()) {
                    root.activeFocusOnTab = true;
                    focus = true;
                }
            }

            leftPadding: textLeftPadding
        }
    }

}
