#include "NodeSelectorLabelColors.h"

#include "megaapi.h"
#include "TokenParserWidgetManager.h"

#include <QCoreApplication>

const QString NodeSelectorLabelColors::sTokens[8] = {
    QString(), // NODE_LBL_UNKNOWN
    QString(QStringLiteral("indicator-pink")), // NODE_LBL_RED
    QString(QStringLiteral("indicator-orange")), // NODE_LBL_ORANGE
    QString(QStringLiteral("indicator-yellow")), // NODE_LBL_YELLOW
    QString(QStringLiteral("indicator-green")), // NODE_LBL_GREEN
    QString(QStringLiteral("indicator-blue")), // NODE_LBL_BLUE
    QString(QStringLiteral("indicator-magenta")), // NODE_LBL_PURPLE
    QString(QStringLiteral("icon-secondary")) // NODE_LBL_GREY
};

const NodeSelectorLabelColors::LabelGradient NodeSelectorLabelColors::sGradients[8] = {
    {QColor(), QColor()}, // NODE_LBL_UNKNOWN
    {QColor(QStringLiteral("#F63D6B")), QColor(QStringLiteral("#FD6F90"))}, // NODE_LBL_RED
    {QColor(QStringLiteral("#FB6514")), QColor(QStringLiteral("#FD853A"))}, // NODE_LBL_ORANGE
    {QColor(QStringLiteral("#DBBB4A")), QColor(QStringLiteral("#F5DA69"))}, // NODE_LBL_YELLOW
    {QColor(QStringLiteral("#09BF5B")), QColor(QStringLiteral("#29DD74"))}, // NODE_LBL_GREEN
    {QColor(QStringLiteral("#05BAF1")), QColor(QStringLiteral("#31D0FE"))}, // NODE_LBL_BLUE
    {QColor(QStringLiteral("#E248C2")), QColor(QStringLiteral("#ED73CC"))}, // NODE_LBL_PURPLE
    {QColor(QStringLiteral("#6E747D")), QColor(QStringLiteral("#888D95"))} // NODE_LBL_GREY
};

QColor NodeSelectorLabelColors::colorForLabel(int label)
{
    if (label <= mega::MegaNode::NODE_LBL_UNKNOWN || label > mega::MegaNode::NODE_LBL_GREY)
    {
        return QColor();
    }

    return TokenParserWidgetManager::instance()->getColor(sTokens[label]);
}

NodeSelectorLabelColors::LabelGradient NodeSelectorLabelColors::gradientForLabel(int label)
{
    if (label <= mega::MegaNode::NODE_LBL_UNKNOWN || label > mega::MegaNode::NODE_LBL_GREY)
    {
        return {QColor(), QColor()};
    }

    return sGradients[label];
}

QVariant NodeSelectorLabelColors::nameForLabel(int label)
{
    switch (label)
    {
        case mega::MegaNode::NODE_LBL_RED:
        {
            return QCoreApplication::translate("NodeSelectorLabelColors", "Red");
        }
        case mega::MegaNode::NODE_LBL_ORANGE:
        {
            return QCoreApplication::translate("NodeSelectorLabelColors", "Orange");
        }
        case mega::MegaNode::NODE_LBL_YELLOW:
        {
            return QCoreApplication::translate("NodeSelectorLabelColors", "Yellow");
        }
        case mega::MegaNode::NODE_LBL_GREEN:
        {
            return QCoreApplication::translate("NodeSelectorLabelColors", "Green");
        }
        case mega::MegaNode::NODE_LBL_BLUE:
        {
            return QCoreApplication::translate("NodeSelectorLabelColors", "Blue");
        }
        case mega::MegaNode::NODE_LBL_PURPLE:
        {
            return QCoreApplication::translate("NodeSelectorLabelColors", "Purple");
        }
        case mega::MegaNode::NODE_LBL_GREY:
        {
            return QCoreApplication::translate("NodeSelectorLabelColors", "Grey");
        }
        case mega::MegaNode::NODE_LBL_UNKNOWN:
        default:
        {
            break;
        }
    }

    return {};
}
