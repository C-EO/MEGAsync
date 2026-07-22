import QtQuick 2.15
import QtQuick.Controls 2.15 as Qml
import QtGraphicalEffects 1.15

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0

Qml.MenuSeparator {
    id: root

    readonly property int horizontalMargin: 4

    padding: 0
    topPadding: 0
    bottomPadding: 0

    contentItem: Rectangle {
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.margins: horizontalMargin
        implicitHeight: Constants.dividerThickness
        color: ColorTheme.surface2
    }
}
