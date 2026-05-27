#include "NodeSelectorDestinationBreadcrumb.h"

#include "NodeSelectorDestinationOverflowPopup.h"
#include "ui_NodeSelectorDestinationBreadcrumb.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QMouseEvent>
#include <QStyle>

namespace
{
constexpr int MAX_VISIBLE_DESTINATION_LEVELS = 4;
constexpr auto SEPARATOR_TEXT = ">";
constexpr auto SEGMENT_INDEX_PROPERTY = "segmentIndex";
}

NodeSelectorDestinationBreadcrumb::NodeSelectorDestinationBreadcrumb(QWidget* parent):
    QFrame(parent),
    ui(new Ui::NodeSelectorDestinationBreadcrumb)
{
    ui->setupUi(this);

    setFrameStyle(QFrame::NoFrame);
    setLineWidth(0);
    setMidLineWidth(0);
    setAttribute(Qt::WA_StyledBackground, true);
    ui->bDestinationOverflow->setAttribute(Qt::WA_StyledBackground, true);
    ui->bDestinationOverflow->setAlignment(Qt::AlignCenter);
    ui->bDestinationOverflow->installEventFilter(this);
    setProperty("displayMode", QLatin1String("destination"));
    ui->bDestinationClear->setVisible(false);
    updateContentVisibility();

    connect(ui->bDestinationClear,
            &QToolButton::clicked,
            this,
            &NodeSelectorDestinationBreadcrumb::clearRequested);
}

NodeSelectorDestinationBreadcrumb::~NodeSelectorDestinationBreadcrumb()
{
    delete ui;
}

void NodeSelectorDestinationBreadcrumb::setPathSegments(const QStringList& segments)
{
    closeOverflowPopup();
    mSegmentsClickable = false;
    mPathSegments = segments;
    rebuildSegments();
    updateContentVisibility();
}

void NodeSelectorDestinationBreadcrumb::setNavigationSegments(const QStringList& segments,
                                                              bool clickable)
{
    closeOverflowPopup();
    mSegmentsClickable = clickable;
    mPathSegments = segments;
    rebuildSegments();
    updateContentVisibility();
}

void NodeSelectorDestinationBreadcrumb::setDisplayMode(DisplayMode mode)
{
    if (mDisplayMode == mode)
    {
        return;
    }

    mDisplayMode = mode;
    setProperty("displayMode",
                mDisplayMode == DisplayMode::DESTINATION ? QLatin1String("destination") :
                                                           QLatin1String("navigation"));
    updateContentVisibility();

    style()->unpolish(this);
    style()->polish(this);
}

void NodeSelectorDestinationBreadcrumb::setTitleText(const QString& text)
{
    ui->lDestinationTitle->setText(text);
}

void NodeSelectorDestinationBreadcrumb::showDefaultUploadOption(bool show)
{
    mShouldShowDefaultUploadOption = show;
    updateContentVisibility();
}

void NodeSelectorDestinationBreadcrumb::setDefaultUploadOption(bool value)
{
    ui->cbAlwaysUploadToLocation->setChecked(value);
}

bool NodeSelectorDestinationBreadcrumb::getDefaultUploadOption() const
{
    return ui->cbAlwaysUploadToLocation->isChecked();
}

bool NodeSelectorDestinationBreadcrumb::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonRelease)
    {
        if (watched == ui->bDestinationOverflow)
        {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton)
            {
                showOverflowPopup();
                return true;
            }
        }

        auto segmentIndex = watched->property(SEGMENT_INDEX_PROPERTY);
        if (segmentIndex.isValid())
        {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton)
            {
                emit segmentActivated(segmentIndex.toInt());
                return true;
            }
        }
    }

    return QFrame::eventFilter(watched, event);
}

void NodeSelectorDestinationBreadcrumb::clearSegmentWidgets()
{
    auto* layout = qobject_cast<QHBoxLayout*>(ui->wDestinationSegments->layout());
    if (!layout)
    {
        return;
    }

    // Keep only the trailing spacer. Dynamic segments, separators and the overflow button
    // are reinserted as needed so navigation mode does not reserve leading space for overflow.
    for (int i = layout->count() - 2; i >= 0; --i)
    {
        auto* item = layout->takeAt(i);
        if (!item)
        {
            continue;
        }
        if (auto* w = item->widget())
        {
            if (w == ui->bDestinationOverflow)
            {
                w->hide();
            }
            else
            {
                w->deleteLater();
            }
        }
        delete item;
    }
}

QWidget* NodeSelectorDestinationBreadcrumb::makeSegmentWidget(const QString& text,
                                                              const bool isFirst,
                                                              const bool isLast,
                                                              const int segmentIndex)
{
    bool highlightCurrentSegment{false};
    if (mDisplayMode == DisplayMode::DESTINATION)
    {
        highlightCurrentSegment = !isFirst && isLast;
    }
    else
    {
        highlightCurrentSegment = isLast;
    }

    auto* label = new QLabel(ui->wDestinationSegments);
    label->setText(text);
    label->setProperty("font-size", QLatin1String("body-2"));
    label->setProperty(highlightCurrentSegment ? "bold" : "regular", true);
    label->setProperty("current", highlightCurrentSegment);
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    label->setContentsMargins(0, 0, 0, 0);
    label->setMargin(0);
    label->setIndent(0);
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    auto font = label->font();
    font.setBold(highlightCurrentSegment);
    label->setFont(font);

    if (mSegmentsClickable)
    {
        label->setCursor(Qt::PointingHandCursor);
        label->setProperty(SEGMENT_INDEX_PROPERTY, segmentIndex);
        label->installEventFilter(this);
    }

    return label;
}

QLabel* NodeSelectorDestinationBreadcrumb::makeSeparatorLabel()
{
    auto* label = new QLabel(ui->wDestinationSegments);
    label->setText(QLatin1String(SEPARATOR_TEXT));
    label->setProperty("font-size", QLatin1String("body-2"));
    label->setProperty("regular", true);
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    label->setContentsMargins(0, 0, 0, 0);
    label->setMargin(0);
    label->setIndent(0);
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    return label;
}

void NodeSelectorDestinationBreadcrumb::updateContentVisibility()
{
    const bool isDestinationMode = mDisplayMode == DisplayMode::DESTINATION;

    ui->lDestinationTitle->setVisible(isDestinationMode);
    ui->cbAlwaysUploadToLocation->setVisible(isDestinationMode && mShouldShowDefaultUploadOption);
    ui->bDestinationClear->setVisible(isDestinationMode && !mPathSegments.isEmpty());

    if (!isDestinationMode)
    {
        ui->wDestinationPath->setMinimumHeight(0);
        if (mSegmentsClickable)
        {
            ui->breadcrumbLayout->setContentsMargins(0, 0, 0, 0);
        }
        else
        {
            ui->breadcrumbLayout->setContentsMargins(0, 12, 0, 12);
        }
    }
}

void NodeSelectorDestinationBreadcrumb::rebuildSegments()
{
    clearSegmentWidgets();

    auto* layout = qobject_cast<QHBoxLayout*>(ui->wDestinationSegments->layout());
    if (!layout)
    {
        return;
    }

    const bool hasOverflow = mPathSegments.size() > MAX_VISIBLE_DESTINATION_LEVELS;
    const auto visibleSegments =
        hasOverflow ? mPathSegments.mid(mPathSegments.size() - MAX_VISIBLE_DESTINATION_LEVELS) :
                      mPathSegments;
    const int visibleStartIndex = mPathSegments.size() - visibleSegments.size();

    // Insert segments before the trailing spacer (the last item in the layout).
    int insertIndex = layout->count() - 1;

    if (hasOverflow)
    {
        ui->bDestinationOverflow->setVisible(true);
        layout->insertWidget(insertIndex++, ui->bDestinationOverflow);

        auto* separator = makeSeparatorLabel();
        layout->insertWidget(insertIndex++, separator);
    }

    const bool isFirst{visibleSegments.size() == 1};

    for (int i = 0; i < visibleSegments.size(); ++i)
    {
        const bool isLast = (i == visibleSegments.size() - 1);
        auto* segmentLabel =
            makeSegmentWidget(visibleSegments.at(i), isFirst, isLast, visibleStartIndex + i);
        layout->insertWidget(insertIndex++, segmentLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);

        if (!isLast)
        {
            auto* separator = makeSeparatorLabel();
            layout->insertWidget(insertIndex++, separator);
        }
    }
}

void NodeSelectorDestinationBreadcrumb::closeOverflowPopup()
{
    if (mOverflowPopup)
    {
        updateOverflowButtonStyle(false);
        mOverflowPopup->close();
        mOverflowPopup = nullptr;
    }
}

void NodeSelectorDestinationBreadcrumb::updateOverflowButtonStyle(bool popupVisible)
{
    ui->bDestinationOverflow->setProperty("popupVisible", popupVisible);
    ui->bDestinationOverflow->style()->unpolish(ui->bDestinationOverflow);
    ui->bDestinationOverflow->style()->polish(ui->bDestinationOverflow);
    ui->bDestinationOverflow->update();
}

void NodeSelectorDestinationBreadcrumb::showOverflowPopup()
{
    if (mPathSegments.size() <= MAX_VISIBLE_DESTINATION_LEVELS)
    {
        return;
    }

    if (mOverflowPopup)
    {
        closeOverflowPopup();
        return;
    }

    // Show only the segments that are NOT already visible in the breadcrumb row.
    const auto hiddenSegments =
        mPathSegments.mid(0, mPathSegments.size() - MAX_VISIBLE_DESTINATION_LEVELS);

    auto* popup = new NodeSelectorDestinationOverflowPopup(ui->wDestinationPath);
    popup->setSegments(hiddenSegments, mSegmentsClickable ? 0 : -1);

    connect(popup,
            &QObject::destroyed,
            this,
            [this]()
            {
                updateOverflowButtonStyle(false);
                mOverflowPopup = nullptr;
            });

    connect(popup,
            &NodeSelectorDestinationOverflowPopup::segmentActivated,
            this,
            [this](int index)
            {
                emit segmentActivated(index);
                closeOverflowPopup();
            });

    mOverflowPopup = popup;
    updateOverflowButtonStyle(true);
    popup->move(ui->wDestinationPath->mapToGlobal(QPoint(0, ui->wDestinationPath->height() + 4)));
    popup->show();
}
