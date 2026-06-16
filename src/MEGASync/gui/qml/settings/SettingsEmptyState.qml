import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.images 1.0
import components.buttons 1.0

/*
  Shared "no items yet" placeholder for the Syncs and Backups settings tabs:
  centered image + title + description + an add button.
*/
ColumnLayout {
    id: root

    property url image
    property string title
    property string description
    property string buttonText

    signal addClicked()

    readonly property int topMargin: 128
    readonly property int imageSize: 160
    readonly property int titlePixelSize: 20
    readonly property int descriptionPixelSize: 16
    readonly property int underImageSpace: 24
    readonly property int textsSpacing: 8
    readonly property int aboveButtonSpacing: 48
    readonly property int leftPaddingAddButton: 12
    readonly property int rightPaddingAddButton: 16

    spacing: 0

    Item {
        Layout.preferredHeight: root.topMargin
        Layout.preferredWidth: parent.width
    }

    SvgImage {
        source: root.image
        sourceSize: Qt.size(root.imageSize, root.imageSize)
        Layout.alignment: Qt.AlignHCenter
    }

    Item {
        Layout.preferredHeight: root.underImageSpace
        Layout.preferredWidth: parent.width
    }

    Text {
        text: root.title
        font.pixelSize: root.titlePixelSize
        font.weight: Font.DemiBold
        color: ColorTheme.textPrimary
        elide: Text.ElideRight
        Layout.alignment: Qt.AlignHCenter
    }

    Item {
        Layout.preferredHeight: root.textsSpacing
        Layout.preferredWidth: parent.width
    }

    Text {
        text: root.description
        font.pixelSize: root.descriptionPixelSize
        font.weight: Font.Normal
        color: ColorTheme.textPrimary
        elide: Text.ElideRight
        Layout.alignment: Qt.AlignHCenter
    }

    Item {
        Layout.preferredHeight: root.aboveButtonSpacing
        Layout.preferredWidth: parent.width
    }

    PrimaryButton {
        text: root.buttonText
        icons.source: Images.plus
        icons.position: Icon.Position.LEFT
        leftPadding: root.leftPaddingAddButton
        rightPadding: root.rightPaddingAddButton
        width: implicitWidth
        onClicked: root.addClicked()
        Layout.alignment: Qt.AlignHCenter
    }
}
