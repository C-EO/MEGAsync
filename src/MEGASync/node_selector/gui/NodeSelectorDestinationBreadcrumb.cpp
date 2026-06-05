#include "NodeSelectorDestinationBreadcrumb.h"

#include "MegaApplication.h"
#include "MegaNodeNames.h"
#include "NodeSelectorDestinationOverflowPopup.h"
#include "ui_NodeSelectorDestinationBreadcrumb.h"
#include "UserAttributesRequests/MyBackupsHandle.h"
#include "Utilities.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QResizeEvent>
#include <QStyle>

namespace
{
constexpr int MAX_VISIBLE_DESTINATION_LEVELS = 4;
constexpr auto SEPARATOR_TEXT = ">";
constexpr int POPUP_HORIZONTAL_OFFSET = -20;
constexpr int POPUP_VERTICAL_GAP = 4;
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
    setProperty("displayMode", QLatin1String("destination"));
    updateContentVisibility();

    connect(ui->bDestinationOverflow,
            &ClickableLabel::clicked,
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

void NodeSelectorDestinationBreadcrumb::setSegments(
    const QList<NodeSelectorBreadcrumbSegment>& segments,
    bool clickable)
{
    if (segments == mSegments && clickable == mSegmentsClickable)
    {
        return;
    }

    mSegmentsClickable = clickable;
    mSegments = segments;
    resolveSegmentNames();
    recalculateSegmentMetrics();
    rebuildSegments();
    updateContentVisibility();
}

QString NodeSelectorDestinationBreadcrumb::resolveSegmentText(
    const NodeSelectorBreadcrumbSegment& segment) const
{
    if (segment.handle != mega::INVALID_HANDLE)
    {
        if (auto* megaApi = MegaSyncApp->getMegaApi())
        {
            std::unique_ptr<mega::MegaNode> node(megaApi->getNodeByHandle(segment.handle));
            if (node)
            {
                const bool isDevice = !QString::fromUtf8(node->getDeviceId()).isEmpty();
                const bool isRoot = node->getType() > mega::MegaNode::TYPE_FOLDER;
                const auto backupsHandle =
                    UserAttributes::MyBackupsHandle::requestMyBackupsHandle()->getMyBackupsHandle();
                const bool isBackupsRoot =
                    backupsHandle != mega::INVALID_HANDLE && segment.handle == backupsHandle;

                // Normal nodes: resolve fresh from the handle so renames are reflected. Root,
                // device and the My Backups folder carry special/translated names that
                // MegaNode::getName() can't produce, so keep the model-resolved text.
                if (!isDevice && !isRoot && !isBackupsRoot)
                {
                    return MegaNodeNames::getNodeName(node.get());
                }
            }
        }
    }

    // Synthetic segments (root labels, empty-state text), deleted nodes, or special nodes
    // (root/device/My Backups) whose translated name is already in the segment text.
    return segment.text;
}

void NodeSelectorDestinationBreadcrumb::resolveSegmentNames()
{
    mPathSegments.clear();
    mPathSegments.reserve(mSegments.size());

    for (const auto& segment: mSegments)
    {
        mPathSegments.push_back(resolveSegmentText(segment));
    }
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

    if (!mPathSegments.isEmpty())
    {
        recalculateSegmentMetrics();
        rebuildSegments();
    }
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
                                                              const int segmentIndex,
                                                              const bool applyInteractivity)
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

    const bool interactive = mSegmentsClickable && applyInteractivity;

    QLabel* label = nullptr;
    if (interactive)
    {
        auto* clickableLabel = new ClickableLabel(ui->wDestinationSegments);
        clickableLabel->setCursor(Qt::PointingHandCursor);
        connect(clickableLabel,
                &ClickableLabel::clicked,
                this,
                [this, segmentIndex]()
                {
                    emit segmentActivated(segmentIndex);
                });
        label = clickableLabel;
    }
    else
    {
        label = new QLabel(ui->wDestinationSegments);
    }

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
    ui->cbContainer->setVisible(isDestinationMode && mShouldShowDefaultUploadOption);
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

void NodeSelectorDestinationBreadcrumb::resizeEvent(QResizeEvent* event)
{
    QFrame::resizeEvent(event);

    if (!mPathSegments.isEmpty() && event->size().width() != event->oldSize().width())
    {
        rebuildSegments(false);
    }
}

void NodeSelectorDestinationBreadcrumb::rebuildSegments(bool force)
{
    const int newStartIndex = calculateVisibleStartIndex();
    if (!force && newStartIndex == mRenderedStartIndex)
    {
        return;
    }

    clearSegmentWidgets();

    auto* layout = qobject_cast<QHBoxLayout*>(ui->wDestinationSegments->layout());
    if (!layout)
    {
        return;
    }

    mVisibleStartIndex = newStartIndex;
    mRenderedStartIndex = newStartIndex;
    const bool hasOverflow = mVisibleStartIndex > 0;
    const auto visibleSegments = mPathSegments.mid(mVisibleStartIndex);
    const int visibleStartIndex = mVisibleStartIndex;

    // Insert segments before the trailing spacer (the last item in the layout).
    int insertIndex = layout->count() - 1;

    if (hasOverflow)
    {
        ui->bDestinationOverflow->setVisible(true);
        layout->insertWidget(insertIndex++, ui->bDestinationOverflow);

        auto* separator = makeSeparatorLabel();
        layout->insertWidget(insertIndex++, separator);
    }

    for (int i = 0; i < visibleSegments.size(); ++i)
    {
        const int segmentIndex = visibleStartIndex + i;
        const bool isFirst = segmentIndex == 0;
        const bool isLast = segmentIndex == mPathSegments.size() - 1;
        auto* segmentLabel =
            makeSegmentWidget(visibleSegments.at(i), isFirst, isLast, segmentIndex);
        layout->insertWidget(insertIndex++, segmentLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);

        if (!isLast)
        {
            auto* separator = makeSeparatorLabel();
            layout->insertWidget(insertIndex++, separator);
        }
    }

    refreshOverflowPopup();
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

void NodeSelectorDestinationBreadcrumb::refreshOverflowPopup()
{
    if (!mOverflowPopup)
    {
        return;
    }

    if (mVisibleStartIndex <= 0)
    {
        closeOverflowPopup();
        return;
    }

    updateOverflowPopupContent();
}

void NodeSelectorDestinationBreadcrumb::updateOverflowPopupContent()
{
    if (!mOverflowPopup)
    {
        return;
    }

    // Resolve the hidden segments' names fresh from their handles at this moment, so the
    // popup reflects renames done while the dialog is open (no node-update listener needed).
    QStringList hiddenSegments;
    hiddenSegments.reserve(mVisibleStartIndex);
    for (int i = 0; i < mVisibleStartIndex && i < mSegments.size(); ++i)
    {
        hiddenSegments.push_back(resolveSegmentText(mSegments.at(i)));
    }

    mOverflowPopup->setSegments(hiddenSegments, mSegmentsClickable ? 0 : -1);

    // Horizontal: anchored to the overflow button (shifted left). Vertical: anchored to the
    // bottom of the path row, not the small 16px button, so the popup clears the whole row.
    const int globalX =
        ui->bDestinationOverflow->mapToGlobal(QPoint(POPUP_HORIZONTAL_OFFSET, 0)).x();
    const int globalY =
        ui->wDestinationPath->mapToGlobal(QPoint(0, ui->wDestinationPath->height())).y() +
        POPUP_VERTICAL_GAP;
    mOverflowPopup->move(globalX, globalY);
}

int NodeSelectorDestinationBreadcrumb::calculateVisibleStartIndex()
{
    const int totalSegments = mPathSegments.size();
    if (totalSegments <= 1)
    {
        return 0;
    }

    const int minimumVisibleStartIndex = qMax(0, totalSegments - MAX_VISIBLE_DESTINATION_LEVELS);
    const int availableWidth = ui->wDestinationSegments->contentsRect().width();

    if (availableWidth <= 0)
    {
        return minimumVisibleStartIndex;
    }

    int visibleStartIndex = minimumVisibleStartIndex;
    while (visibleStartIndex < totalSegments - 1 &&
           calculateRequiredWidthForVisibleStartIndex(visibleStartIndex) > availableWidth)
    {
        ++visibleStartIndex;
    }

    return visibleStartIndex;
}

int NodeSelectorDestinationBreadcrumb::calculateRequiredWidthForVisibleStartIndex(
    int visibleStartIndex)
{
    auto* layout = qobject_cast<QHBoxLayout*>(ui->wDestinationSegments->layout());
    if (!layout)
    {
        return 0;
    }

    int requiredWidth = 0;
    int itemCount = 0;

    if (visibleStartIndex > 0)
    {
        requiredWidth += mOverflowWidth;
        requiredWidth += mSeparatorWidth;
        itemCount += 2;
    }

    for (int segmentIndex = visibleStartIndex; segmentIndex < mPathSegments.size(); ++segmentIndex)
    {
        requiredWidth += mSegmentWidths.value(segmentIndex);
        ++itemCount;

        if (segmentIndex != mPathSegments.size() - 1)
        {
            requiredWidth += mSeparatorWidth;
            ++itemCount;
        }
    }

    if (itemCount > 1)
    {
        requiredWidth += (itemCount - 1) * layout->spacing();
    }

    return requiredWidth;
}

void NodeSelectorDestinationBreadcrumb::recalculateSegmentMetrics()
{
    mSegmentWidths.clear();
    mSegmentWidths.reserve(mPathSegments.size());

    for (int i = 0; i < mPathSegments.size(); ++i)
    {
        auto* segmentWidget =
            makeSegmentWidget(mPathSegments.at(i), i == 0, i == mPathSegments.size() - 1, i, false);
        segmentWidget->ensurePolished();
        mSegmentWidths.push_back(segmentWidget->sizeHint().width());
        delete segmentWidget;
    }

    auto* separator = makeSeparatorLabel();
    separator->ensurePolished();
    mSeparatorWidth = separator->sizeHint().width();
    delete separator;

    ui->bDestinationOverflow->ensurePolished();
    mOverflowWidth = ui->bDestinationOverflow->sizeHint().width();
}

void NodeSelectorDestinationBreadcrumb::showOverflowPopup()
{
    if (mVisibleStartIndex <= 0)
    {
        return;
    }

    if (mOverflowPopup)
    {
        closeOverflowPopup();
        return;
    }

    auto* popup = new NodeSelectorDestinationOverflowPopup(this);

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
    // Show first, then size + position: a top-level Qt::Popup may ignore a move() issued
    // before it is shown (notably on macOS), which would leave it mispositioned.
    popup->show();
    updateOverflowPopupContent();
}
