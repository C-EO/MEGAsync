import QtQuick 2.15

import common 1.0
import components.texts 1.0 as Texts
import AccountStateQuickWidget 1.0

Rectangle {
    id: root

    property var segment: null
    property var childSegment: null
    property var childSegmentFillColor
    property string childTooltipText: ""
    property int minVisibleChildSegmentWidth: 2
    property bool roundLeftEdge: false
    property bool roundRightEdge: false
    property int segmentRadius: 4
    property string tooltipText: ""
    property color sizeTextColor: ColorTheme.textPrimary

    readonly property bool isFreeSegment: Number(root.segment && root.segment.type) === AccountStateQuickWidget.Free
    readonly property string centeredSizeText: root.segment && root.segment.sizeText
                                              ? root.segment.sizeText
                                              : ""
    readonly property bool showCenteredSizeText: root.isFreeSegment
                                                 && root.centeredSizeText.length > 0
                                                 && root.width >= centeredSizeTextLabel.implicitWidth + 16

    radius: (roundLeftEdge || roundRightEdge) ? segmentRadius : 0
    clip: true

    function childSegmentWidth() {
        const parentValue = Number(root.segment && root.segment.value)
        const childValue = Number(root.childSegment && root.childSegment.value)
        if (childValue <= 0) {
            return 0
        }

        // The parent segment's width is laid out against max(parentValue, childValue),
        // so the overlay uses the same denominator to stay in sync.
        const effectiveParentValue = Math.max(parentValue, childValue)
        if (effectiveParentValue <= 0) {
            return 0
        }

        const proportionalWidth = root.width * childValue / effectiveParentValue
        const minimumWidth = Math.max(0, root.minVisibleChildSegmentWidth)
        // When child >= parent and the parent has value of its own, leave at least
        // minimumWidth px of the parent visible. If parentValue is 0 there is nothing
        // to preserve, so the overlay is allowed to fill the whole segment.
        const maximumWidth = (parentValue > 0 && childValue >= parentValue)
                             ? root.width - minimumWidth
                             : root.width
        return Math.min(maximumWidth, Math.max(minimumWidth, proportionalWidth))
    }

    // TODO: Remove these corner-hiding helper rectangles when we move to Qt 6,
    // which supports setting the radius per corner directly.
    Rectangle {
        id: hidesRightProgressBarRoundedCorners

        anchors {
            top: parent.top
            right: parent.right
            bottom: parent.bottom
        }
        width: parent.roundLeftEdge && !parent.roundRightEdge
               ? Math.max(0, parent.width - parent.segmentRadius)
               : 0
        visible: parent.roundLeftEdge
                 && !parent.roundRightEdge
                 && parent.width > parent.segmentRadius
        color: parent.color
    }

    Rectangle {
        id: hidesLeftProgressBarRoundedCorners

        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }
        width: parent.roundRightEdge && !parent.roundLeftEdge
               ? Math.max(0, parent.width - parent.segmentRadius)
               : 0
        visible: parent.roundRightEdge
                 && !parent.roundLeftEdge
                 && parent.width > parent.segmentRadius
        color: parent.color
    }

    Texts.Text {
        id: centeredSizeTextLabel

        anchors.centerIn: parent
        visible: root.showCenteredSizeText
        text: root.centeredSizeText
        color: root.sizeTextColor
        font.pixelSize: 10
        font.weight: Font.DemiBold
        lineHeight: 16
        lineHeightMode: Text.FixedHeight
        verticalAlignment: Text.AlignVCenter
        z: 1
    }

    Item {
        id: childOverlayContainer

        readonly property bool overlayCoversLeftEdge: root.roundLeftEdge
                                                      && root.childSegmentWidth() >= root.width
        readonly property bool overlayCoversRightEdge: root.roundRightEdge

        anchors {
            top: parent.top
            right: parent.right
            bottom: parent.bottom
        }
        z: 1
        width: root.childSegmentWidth()
        visible: width > 0
        clip: true

        Rectangle {
            id: childOverlayFill

            anchors.fill: parent
            radius: (childOverlayContainer.overlayCoversLeftEdge
                     || childOverlayContainer.overlayCoversRightEdge)
                    ? root.segmentRadius
                    : 0
            color: root.childSegmentFillColor && root.childSegment
                   ? root.childSegmentFillColor(root.childSegment)
                   : "transparent"

            // TODO: Remove these corner-hiding helper rectangles when we move to Qt 6,
            // which supports setting the radius per corner directly.
            Rectangle {
                id: hidesChildOverlayRightCorners

                anchors {
                    top: parent.top
                    right: parent.right
                    bottom: parent.bottom
                }
                width: childOverlayContainer.overlayCoversLeftEdge
                       && !childOverlayContainer.overlayCoversRightEdge
                       ? Math.max(0, parent.width - root.segmentRadius)
                       : 0
                visible: width > 0
                color: parent.color
            }

            Rectangle {
                id: hidesChildOverlayLeftCorners

                anchors {
                    left: parent.left
                    top: parent.top
                    bottom: parent.bottom
                }
                width: childOverlayContainer.overlayCoversRightEdge
                       && !childOverlayContainer.overlayCoversLeftEdge
                       ? Math.max(0, parent.width - root.segmentRadius)
                       : 0
                visible: width > 0
                color: parent.color
            }
        }

        MouseArea {
            id: childMouseArea

            anchors.fill: parent
            hoverEnabled: true
            enabled: root.childTooltipText.length > 0

            QuotaProgressToolTip {
                id: childSegmentToolTip

                visible: parent.containsMouse && parent.enabled
                text: root.childTooltipText
                anchorItem: childOverlayContainer
            }
        }
    }

    MouseArea {
        id: parentMouseArea

        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }
        width: (childOverlayContainer.visible && root.childTooltipText.length > 0)
               ? Math.max(0, root.width - childOverlayContainer.width)
               : root.width
        hoverEnabled: true

        QuotaProgressToolTip {
            id: parentSegmentToolTip

            visible: parent.containsMouse
            text: root.tooltipText
            anchorItem: root
        }
    }
}
