#include "BreadcrumbSegment.h"

#include <QStyle>

BreadcrumbSegment::BreadcrumbSegment(QWidget* parent):
    ClickableLabel(parent)
{
    setProperty("font-size", QLatin1String("body-2"));
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    setContentsMargins(0, 0, 0, 0);
    setMargin(0);
    setIndent(0);
    setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
}

void BreadcrumbSegment::setHighlighted(bool highlighted)
{
    setProperty("bold", highlighted);
    setProperty("regular", !highlighted);
    setProperty("current", highlighted);

    auto segmentFont = font();
    segmentFont.setBold(highlighted);
    setFont(segmentFont);

    style()->unpolish(this);
    style()->polish(this);
}

void BreadcrumbSegment::setInteractive(bool interactive)
{
    setCursor(interactive ? Qt::PointingHandCursor : Qt::ArrowCursor);
}
