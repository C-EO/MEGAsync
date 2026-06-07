#include "DestinationBreadcrumb.h"

#include "Breadcrumb.h"
#include "BreadcrumbSegment.h"
#include "ui_DestinationBreadcrumb.h"

#include <QToolButton>

DestinationBreadcrumb::DestinationBreadcrumb(QWidget* parent):
    QFrame(parent),
    ui(new Ui::DestinationBreadcrumb)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true);

    // Destination segments are static text; the current folder (last, unless it is the lone root)
    // is the only highlighted one.
    ui->breadcrumb->setSegmentFactory(
        [](const QString& text, int index, bool isFirst, bool isLast) -> QWidget*
        {
            Q_UNUSED(index)
            auto* segment = new BreadcrumbSegment;
            segment->setText(text);
            segment->setHighlighted(!isFirst && isLast);
            segment->setInteractive(false);
            return segment;
        });

    connect(ui->bClear, &QToolButton::clicked, this, &DestinationBreadcrumb::clearRequested);

    updateContentVisibility();
}

DestinationBreadcrumb::~DestinationBreadcrumb()
{
    delete ui;
}

void DestinationBreadcrumb::setSegments(const QList<NodeSelectorBreadcrumbSegment>& segments)
{
    mHasSegments = !segments.isEmpty();
    ui->breadcrumb->setSegments(segments);
    updateContentVisibility();
}

void DestinationBreadcrumb::setTitleText(const QString& text)
{
    ui->lTitle->setText(text);
}

void DestinationBreadcrumb::showDefaultUploadOption(bool show)
{
    mShouldShowDefaultUploadOption = show;
    updateContentVisibility();
}

void DestinationBreadcrumb::setDefaultUploadOption(bool value)
{
    ui->cbAlwaysUploadToLocation->setChecked(value);
}

bool DestinationBreadcrumb::getDefaultUploadOption() const
{
    return ui->cbAlwaysUploadToLocation->isChecked();
}

void DestinationBreadcrumb::updateContentVisibility()
{
    ui->cbContainer->setVisible(mShouldShowDefaultUploadOption);
    ui->bClear->setVisible(mHasSegments);
}
