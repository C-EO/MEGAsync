import QtQuick 2.15

Item {
    id: root

    property var segments: []
    property var segmentFillColor
    property var tooltipTextForSegment
    property bool shouldRoundLastSegment: true
    property int tightSpacing: 2
    property int segmentRadius: 4
    property int minVisibleSegmentWidth: 4
    property int minVisibleChildSegmentWidth: Math.max(1, Math.floor(minVisibleSegmentWidth / 2))

    function sumOfChildValues(segment) {
        if (!segment || !segment.children) {
            return 0
        }
        return segment.children.reduce(function(total, child) {
            return total + Math.max(0, Number(child && child.value))
        }, 0)
    }

    function effectiveSegmentValue(segment) {
        if (!segment) {
            return 0
        }
        return Math.max(Number(segment.value), root.sumOfChildValues(segment))
    }

    function totalVisibleValue() {
        return (segments || []).reduce(function(total, segment) {
            return total + root.effectiveSegmentValue(segment)
        }, 0)
    }

    function availableSegmentWidth() {
        return Math.max(0,
                        root.width
                        - progressSegmentsLayout.spacing * (visibleRepeater.count - 1))
    }

    function normalizedWidthForSegment(segment) {
        const total = root.totalVisibleValue()
        return total > 0 ? root.effectiveSegmentValue(segment) / total : 0
    }

    function computeSegmentWidths() {
        const segmentItems = segments || []
        const count = segmentItems.length
        if (count === 0) {
            return []
        }

        const availableWidth = root.availableSegmentWidth()
        if (availableWidth <= 0) {
            return segmentItems.map(function() {
                return 0
            })
        }

        const minimumWidth = Math.max(0, root.minVisibleSegmentWidth)
        let widths = segmentItems.map(function(segment) {
            return Math.max(0,
                            Math.max(minimumWidth,
                                     availableWidth * root.normalizedWidthForSegment(segment)))
        })

        const totalWidth = widths.reduce(function(total, currentWidth) {
            return total + currentWidth
        }, 0)
        const overflow = totalWidth - availableWidth

        if (overflow > 0) {
            let largestIndex = 0
            for (let i = 1; i < widths.length; ++i) {
                if (widths[i] > widths[largestIndex]) {
                    largestIndex = i
                }
            }

            widths[largestIndex] = Math.max(minimumWidth, widths[largestIndex] - overflow)
        }

        return widths
    }

    function widthForSegment(index, segment) {
        const widths = root.computeSegmentWidths()
        if (index < 0 || index >= widths.length) {
            return 0
        }
        return widths[index]
    }

    Row {
        id: progressSegmentsLayout

        anchors.fill: parent
        spacing: visibleRepeater.count > 1 ? root.tightSpacing : 0

        Repeater {
            id: visibleRepeater
            model: root.segments || []

            QuotaProgressSegmentItem {
                required property var modelData
                required property int index

                readonly property bool shouldRoundRightEdge: index === visibleRepeater.count - 1
                                                             && root.shouldRoundLastSegment

                segment: modelData
                width: root.widthForSegment(index, modelData)
                height: root.height
                color: root.segmentFillColor ? root.segmentFillColor(modelData) : "transparent"
                childSegment: modelData.children && modelData.children.length > 0
                              ? modelData.children[0]
                              : null
                childSegmentFillColor: root.segmentFillColor
                childTooltipText: root.tooltipTextForSegment && modelData.children
                                  && modelData.children.length > 0
                                  ? root.tooltipTextForSegment(modelData.children[0])
                                  : ""
                minVisibleChildSegmentWidth: root.minVisibleChildSegmentWidth
                roundLeftEdge: index === 0
                roundRightEdge: shouldRoundRightEdge
                segmentRadius: root.segmentRadius
                tooltipText: root.tooltipTextForSegment ? root.tooltipTextForSegment(modelData) : ""
            }
        }
    }
}
