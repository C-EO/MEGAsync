import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

Item {
    id: root

    readonly property int defaultTopMargin: 19

    Column {
        id: content

        anchors.fill: parent
        anchors.topMargin: defaultTopMargin
    }
}
