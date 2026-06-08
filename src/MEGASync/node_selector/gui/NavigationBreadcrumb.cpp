#include "NavigationBreadcrumb.h"

#include "Breadcrumb.h"
#include "BreadcrumbSegment.h"
#include "NavigationBreadcrumbLastSegment.h"
#include "ui_NavigationBreadcrumb.h"

NavigationBreadcrumb::NavigationBreadcrumb(QWidget* parent):
    QFrame(parent),
    ui(new Ui::NavigationBreadcrumb)
{
    ui->setupUi(this);

    ui->breadcrumb->setSegmentFactory(
        [this](const QString& text, int index, bool isFirst, bool isLast) -> QWidget*
        {
            Q_UNUSED(isFirst)

            // The last segment carries the chevron + context menu.
            if (isLast)
            {
                // A read-only root has no context-menu actions, so render the last segment as a
                // plain highlighted segment without the chevron + menu affordance.
                if (mCurrentRootReadOnly)
                {
                    auto* segment = new BreadcrumbSegment;
                    segment->setText(text);
                    segment->setHighlighted(true);
                    segment->setInteractive(false);
                    return segment;
                }

                auto* lastSegment = new NavigationBreadcrumbLastSegment;
                lastSegment->setText(text);
                lastSegment->setHighlighted(true);

                connect(lastSegment,
                        &NavigationBreadcrumbLastSegment::clicked,
                        this,
                        [this, index]()
                        {
                            emit segmentActivated(index);
                        });
                connect(lastSegment,
                        &NavigationBreadcrumbLastSegment::menuRequested,
                        this,
                        &NavigationBreadcrumb::lastSegmentMenuRequested);

                return lastSegment;
            }

            auto* segment = new BreadcrumbSegment;
            segment->setText(text);
            segment->setHighlighted(false);
            segment->setInteractive(true);
            connect(segment,
                    &ClickableLabel::clicked,
                    this,
                    [this, index]()
                    {
                        emit segmentActivated(index);
                    });
            return segment;
        });

    connect(ui->breadcrumb,
            &Breadcrumb::overflowSegmentActivated,
            this,
            &NavigationBreadcrumb::segmentActivated);
}

NavigationBreadcrumb::~NavigationBreadcrumb()
{
    delete ui;
}

void NavigationBreadcrumb::setSegments(const QList<NodeSelectorBreadcrumbSegment>& segments,
                                       bool currentRootReadOnly)
{
    mCurrentRootReadOnly = currentRootReadOnly;
    ui->breadcrumb->setSegments(segments, true);
}
