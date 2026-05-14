#ifndef NODESELECTORLABELCOLORS_H
#define NODESELECTORLABELCOLORS_H

#include <QColor>

class NodeSelectorLabelColors
{
public:
    struct LabelGradient
    {
        QColor from;
        QColor to;
    };

    NodeSelectorLabelColors() = delete;

    /// Returns the solid color for the given label index.
    /// `label` is expected to match `mega::MegaNode::NODE_LBL_*` constants.
    /// Returns an invalid QColor when the label is NODE_LBL_UNKNOWN or out of range.
    static QColor colorForLabel(int label);

    /// Returns the two-tone gradient (bottom-to-top) for the given label index.
    /// Returns invalid colors when the label is NODE_LBL_UNKNOWN or out of range.
    static LabelGradient gradientForLabel(int label);

private:
    // Indexed by mega::MegaNode::NODE_LBL_* (0 = UNKNOWN, 7 = GREY).
    static const QString sTokens[8];
    static const LabelGradient sGradients[8];
};

#endif // NODESELECTORLABELCOLORS_H
