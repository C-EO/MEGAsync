import QtQuick 2.15

import common 1.0

import components.images 1.0
import components.texts 1.0 as Texts

import ServiceUrls 1.0

Item {
    id: root

    property alias icon: hintIcon.source
    property alias title: hintTitle.rawText
    property alias text: hintText.rawText
    property alias iconColor: hintIcon.color
    property alias titleColor: hintTitle.color
    property alias textColor: hintText.color
    property alias textSpacing: textColumn.spacing

    property int type: Constants.MessageType.NONE
    property int textSize: Texts.Text.Size.NORMAL
    property bool colorizeText: true

    readonly property color _typeColor: {
        switch (type) {
            case Constants.MessageType.WARNING: return ColorTheme.textWarning;
            case Constants.MessageType.ERROR:   return ColorTheme.textError;
            default: return enabled ? ColorTheme.textPrimary : ColorTheme.textDisabled;
        }
    }
    readonly property url _typeIcon: {
        switch (type) {
            case Constants.MessageType.WARNING: return Images.alertTriangle;
            case Constants.MessageType.ERROR:   return Images.xCircle;
            default: return "";
        }
    }

    implicitHeight: mainRow.height

    Row {
        id: mainRow

        height: root.visible ? textColumn.implicitHeight : 0
        spacing: root.icon !== "" ? 8 : 0
        width: root.width

        SvgImage {
            id: hintIcon

            source: root._typeIcon
            color: root._typeColor
            sourceSize: Qt.size(16, 16)
            opacity: enabled ? 1.0 : 0.2
        }

        Column {
            id: textColumn

            anchors.top: parent.top
            width: mainRow.width - hintIcon.width - mainRow.spacing

            Texts.RichText {
                id: hintTitle

                color: root.colorizeText
                       ? root._typeColor
                       : (enabled ? ColorTheme.textPrimary : ColorTheme.textDisabled)
                height: rawText !== "" ? implicitHeight : 0
                width: parent.width
                opacity: enabled ? 1.0 : 0.2
                wrapMode: Text.WordWrap
                font{
                    bold: true
                    pixelSize: root.textSize
                }
            }

            Texts.RichText {
                id: hintText

                color: root.colorizeText
                       ? root._typeColor
                       : (enabled ? ColorTheme.textPrimary : ColorTheme.textDisabled)
                height: rawText !== "" ? implicitHeight : 0
                width: parent.width
                opacity: enabled ? 1.0 : 0.2
                font.pixelSize: root.textSize
                wrapMode: Text.WordWrap
                url: serviceUrlsAccess.getContactSupportUrl()
                manageMouse: true
            }
        }

    } // Row: mainRow

}
