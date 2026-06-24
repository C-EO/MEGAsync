#include "BreadcrumbSegment.h"

#include <QStyle>

BreadcrumbSegment::BreadcrumbSegment(QWidget* parent):
    ClickableLabel(parent)
{
    setProperty("font-size", QLatin1String("body-2"));
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    setContentsMargins(8, 3, 8, 3);
    setMargin(0);
    setIndent(0);
    setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
}

void BreadcrumbSegment::setHighlighted(bool highlighted)
{
    if (highlighted)
    {
        setProperty("bold", true);
        setProperty("regular", QVariant());
    }
    else
    {
        setProperty("regular", false);
        setProperty("bold", QVariant());
    }

    setProperty("current", highlighted);

    auto segmentFont = font();
    segmentFont.setBold(highlighted);
    setFont(segmentFont);

    style()->unpolish(this);
    style()->polish(this);
}

void BreadcrumbSegment::setInteractive(bool interactive)
{
    setEnterCursorOverride(interactive);
    setCursor(interactive ? Qt::PointingHandCursor : Qt::ArrowCursor);
}

void BreadcrumbSegment::setFirst(bool first)
{
    // Every segment is padded 8,3,8,3; the first one drops its left padding so the path
    // starts flush against the breadcrumb's left edge.
    setContentsMargins(first ? 0 : 8, 3, 8, 3);
}
