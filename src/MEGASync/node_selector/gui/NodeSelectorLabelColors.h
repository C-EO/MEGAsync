#ifndef NODESELECTORLABELCOLORS_H
#define NODESELECTORLABELCOLORS_H

#include <QColor>

class NodeSelectorLabelColors
{
public:
    NodeSelectorLabelColors() = delete;

    /// Returns the color for the given label index in the current theme.
    /// `label` is expected to match `mega::MegaNode::NODE_LBL_*` constants.
    /// Returns an invalid QColor when the label is NODE_LBL_UNKNOWN or out of range.
    static QColor colorForLabel(int label);

private:
    // Placeholder palette. Replace with the final design-system tokens once available.
    // Indexed by mega::MegaNode::NODE_LBL_* (0 = UNKNOWN, 7 = GREY).
    static const QColor sDarkColors[8];
    static const QColor sLightColors[8];
};

#endif // NODESELECTORLABELCOLORS_H
