#include "NodeSelectorDestinationBreadcrumb.h"

#include "NodeSelectorDestinationOverflowPopup.h"
#include "ui_NodeSelectorDestinationBreadcrumb.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QPushButton>
#include <QStyle>
#include <QToolButton>

namespace
{
constexpr int MAX_VISIBLE_DESTINATION_LEVELS = 4;
constexpr auto SEPARATOR_TEXT = ">";
}

NodeSelectorDestinationBreadcrumb::NodeSelectorDestinationBreadcrumb(QWidget* parent):
    QFrame(parent),
    ui(new Ui::NodeSelectorDestinationBreadcrumb)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground, true);
    setProperty("displayMode", QLatin1String("destination"));
    ui->bDestinationClear->setVisible(false);
    updateContentVisibility();

    connect(ui->bDestinationOverflow,
            &QPushButton::clicked,
            this,
            &NodeSelectorDestinationBreadcrumb::showOverflowPopup);

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

void NodeSelectorDestinationBreadcrumb::setNavigationSegments(const QStringList& segments)
{
    closeOverflowPopup();
    mSegmentsClickable = true;
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

void NodeSelectorDestinationBreadcrumb::clearSegmentWidgets()
{
    auto* layout = qobject_cast<QHBoxLayout*>(ui->wDestinationSegments->layout());
    if (!layout)
    {
        return;
    }

    // Remove every item except the overflow button (index 0) and the trailing spacer.
    // The breadcrumb owns those two; only the dynamic segments/separators are destroyed.
    for (int i = layout->count() - 2; i >= 1; --i)
    {
        auto* item = layout->takeAt(i);
        if (!item)
        {
            continue;
        }
        if (auto* w = item->widget())
        {
            w->deleteLater();
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

    if (!mSegmentsClickable)
    {
        auto* label = new QLabel(ui->wDestinationSegments);
        label->setText(text);
        label->setProperty("font-size", QLatin1String("body-2"));
        label->setProperty(highlightCurrentSegment ? "bold" : "regular", true);
        return label;
    }

    auto* button = new QToolButton(ui->wDestinationSegments);
    button->setText(text);
    button->setToolButtonStyle(Qt::ToolButtonTextOnly);
    button->setAutoRaise(true);
    button->setCursor(Qt::PointingHandCursor);
    button->setProperty("font-size", QLatin1String("body-2"));
    button->setProperty("regular", !highlightCurrentSegment);
    button->setProperty("current", highlightCurrentSegment);

    auto font = button->font();
    font.setBold(highlightCurrentSegment);
    button->setFont(font);

    connect(button,
            &QToolButton::clicked,
            this,
            [this, segmentIndex]()
            {
                emit segmentActivated(segmentIndex);
            });

    return button;
}

QLabel* NodeSelectorDestinationBreadcrumb::makeSeparatorLabel()
{
    auto* label = new QLabel(ui->wDestinationSegments);
    label->setText(QLatin1String(SEPARATOR_TEXT));
    label->setProperty("font-size", QLatin1String("body-2"));
    label->setProperty("regular", true);
    return label;
}

void NodeSelectorDestinationBreadcrumb::updateContentVisibility()
{
    const bool isDestinationMode = mDisplayMode == DisplayMode::DESTINATION;

    ui->lDestinationTitle->setVisible(isDestinationMode);
    ui->cbAlwaysUploadToLocation->setVisible(isDestinationMode && mShouldShowDefaultUploadOption);
    ui->bDestinationClear->setVisible(isDestinationMode && !mPathSegments.isEmpty());
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

    ui->bDestinationOverflow->setVisible(hasOverflow);

    // Insert segments before the trailing spacer (the last item in the layout).
    int insertIndex = layout->count() - 1;

    if (hasOverflow)
    {
        auto* separator = makeSeparatorLabel();
        layout->insertWidget(insertIndex++, separator);
    }

    const bool isFirst{visibleSegments.size() == 1};

    for (int i = 0; i < visibleSegments.size(); ++i)
    {
        const bool isLast = (i == visibleSegments.size() - 1);
        auto* segmentLabel =
            makeSegmentWidget(visibleSegments.at(i), isFirst, isLast, visibleStartIndex + i);
        layout->insertWidget(insertIndex++, segmentLabel);

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
        mOverflowPopup->close();
        mOverflowPopup = nullptr;
    }
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
    popup->move(ui->wDestinationPath->mapToGlobal(QPoint(0, ui->wDestinationPath->height() + 4)));
    popup->show();
}
