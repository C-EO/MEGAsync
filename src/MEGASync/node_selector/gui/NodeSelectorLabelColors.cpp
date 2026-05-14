#include "NodeSelectorLabelColors.h"

#include "megaapi.h"
#include "Preferences/Preferences.h"
#include "ThemeManager.h"

const QColor NodeSelectorLabelColors::sDarkColors[8] = {
    QColor(), // NODE_LBL_UNKNOWN
    QColor(QStringLiteral("#FF6B6B")), // NODE_LBL_RED
    QColor(QStringLiteral("#FFA94D")), // NODE_LBL_ORANGE
    QColor(QStringLiteral("#FFD43B")), // NODE_LBL_YELLOW
    QColor(QStringLiteral("#51CF66")), // NODE_LBL_GREEN
    QColor(QStringLiteral("#4DABF7")), // NODE_LBL_BLUE
    QColor(QStringLiteral("#CC5DE8")), // NODE_LBL_PURPLE
    QColor(QStringLiteral("#909296")) // NODE_LBL_GREY
};

const QColor NodeSelectorLabelColors::sLightColors[8] = {
    QColor(), // NODE_LBL_UNKNOWN
    QColor(QStringLiteral("#E03131")), // NODE_LBL_RED
    QColor(QStringLiteral("#E8590C")), // NODE_LBL_ORANGE
    QColor(QStringLiteral("#F08C00")), // NODE_LBL_YELLOW
    QColor(QStringLiteral("#2F9E44")), // NODE_LBL_GREEN
    QColor(QStringLiteral("#1971C2")), // NODE_LBL_BLUE
    QColor(QStringLiteral("#9C36B5")), // NODE_LBL_PURPLE
    QColor(QStringLiteral("#5C5F66")) // NODE_LBL_GREY
};

QColor NodeSelectorLabelColors::colorForLabel(int label)
{
    if (label <= mega::MegaNode::NODE_LBL_UNKNOWN || label > mega::MegaNode::NODE_LBL_GREY)
    {
        return QColor();
    }

    const bool darkMode =
        ThemeManager::instance()->currentColorScheme() == Preferences::ThemeAppeareance::DARK;
    return darkMode ? sDarkColors[label] : sLightColors[label];
}
