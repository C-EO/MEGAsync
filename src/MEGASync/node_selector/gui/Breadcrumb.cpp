#include "Breadcrumb.h"

#include "BreadcrumbSegment.h"
#include "MegaApplication.h"
#include "MegaNodeNames.h"
#include "NodeSelectorDestinationOverflowPopup.h"
#include "TokenizableItems/IconLabel.h"
#include "ui_Breadcrumb.h"
#include "UserAttributesRequests/MyBackupsHandle.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QResizeEvent>
#include <QStyle>

namespace
{
constexpr int MAX_VISIBLE_DESTINATION_LEVELS = 4;
constexpr int POPUP_HORIZONTAL_OFFSET = -20;
constexpr int POPUP_VERTICAL_GAP = 4;
}

Breadcrumb::Breadcrumb(QWidget* parent):
    QFrame(parent),
    ui(new Ui::Breadcrumb)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground, true);
    ui->bOverflow->setAttribute(Qt::WA_StyledBackground, true);
    ui->bOverflow->setAlignment(Qt::AlignCenter);

    connect(ui->bOverflow, &ClickableLabel::clicked, this, &Breadcrumb::showOverflowPopup);
}

Breadcrumb::~Breadcrumb()
{
    delete ui;
}

void Breadcrumb::setSegmentFactory(SegmentFactory factory)
{
    mSegmentFactory = std::move(factory);
    if (!mPathSegments.isEmpty())
    {
        recalculateSegmentMetrics();
        rebuildSegments();
    }
}

void Breadcrumb::setSegments(const QList<NodeSelectorBreadcrumbSegment>& segments,
                             bool overflowSegmentsNavigable)
{
    mOverflowSegmentsNavigable = overflowSegmentsNavigable;

    if (segments == mSegments)
    {
        return;
    }

    mSegments = segments;
    resolveSegmentNames();
    recalculateSegmentMetrics();
    rebuildSegments();
}

void Breadcrumb::clear()
{
    mSegments.clear();
    rebuildSegments(true);
}

void Breadcrumb::onNodesRenamed(const QList<mega::MegaHandle>& handles)
{
    // A rename only changes a segment's display text, not the path. Ask for a refresh when any
    // renamed node is one of the segments currently shown.
    if (containsAnyHandle(handles))
    {
        emit refreshNeeded();
    }
}

bool Breadcrumb::containsAnyHandle(const QList<mega::MegaHandle>& handles) const
{
    for (const auto& segment: mSegments)
    {
        if (segment.handle != mega::INVALID_HANDLE && handles.contains(segment.handle))
        {
            return true;
        }
    }

    return false;
}

QString Breadcrumb::resolveSegmentText(const NodeSelectorBreadcrumbSegment& segment) const
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

void Breadcrumb::resolveSegmentNames()
{
    mPathSegments.clear();
    mPathSegments.reserve(mSegments.size());

    for (const auto& segment: std::as_const(mSegments))
    {
        mPathSegments.push_back(resolveSegmentText(segment));
    }
}

void Breadcrumb::clearSegmentWidgets()
{
    auto* layout = qobject_cast<QHBoxLayout*>(ui->wSegments->layout());
    if (!layout)
    {
        return;
    }

    // Keep only the trailing spacer. Dynamic segments, separators and the overflow button
    // are reinserted as needed so navigation does not reserve leading space for overflow.
    for (int i = layout->count() - 2; i >= 0; --i)
    {
        auto* item = layout->takeAt(i);
        if (!item)
        {
            continue;
        }
        if (auto* w = item->widget())
        {
            if (w == ui->bOverflow)
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

QWidget* Breadcrumb::createSegmentWidget(const QString& text,
                                         const int index,
                                         const bool isFirst,
                                         const bool isLast)
{
    QWidget* widget = mSegmentFactory ? mSegmentFactory(text, index, isFirst, isLast) : nullptr;

    if (!widget)
    {
        // Fallback: a plain, non-interactive segment.
        auto* segment = new BreadcrumbSegment;
        segment->setText(text);
        segment->setHighlighted(isLast);
        segment->setFirst(isFirst);
        widget = segment;
    }

    widget->setParent(ui->wSegments);
    return widget;
}

QWidget* Breadcrumb::makeSeparatorLabel()
{
    auto* label = new IconLabel(ui->wSegments);
    label->setIcon(Utilities::getIcon(QLatin1String("chevron-right"),
                                      Utilities::AttributeType::SMALL |
                                          Utilities::AttributeType::THIN |
                                          Utilities::AttributeType::OUTLINE));
    label->setProperty(TOKEN_PROPERTIES::normalOff, QLatin1String("icon-secondary"));
    label->setProperty("font-size", QLatin1String("body-2"));
    label->setProperty("regular", true);
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    label->setContentsMargins(0, 0, 0, 0);
    label->setIconSize(QSize(16, 16));
    return label;
}

void Breadcrumb::resizeEvent(QResizeEvent* event)
{
    QFrame::resizeEvent(event);

    if (!mPathSegments.isEmpty() && event->size().width() != event->oldSize().width())
    {
        rebuildSegments(false);
    }
}

void Breadcrumb::rebuildSegments(bool force)
{
    const int newStartIndex = calculateVisibleStartIndex();
    if (!force && newStartIndex == mRenderedStartIndex)
    {
        return;
    }

    clearSegmentWidgets();

    auto* layout = qobject_cast<QHBoxLayout*>(ui->wSegments->layout());
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
        ui->bOverflow->setVisible(true);
        layout->insertWidget(insertIndex++, ui->bOverflow);

        auto* separator = makeSeparatorLabel();
        layout->insertWidget(insertIndex++, separator);
    }

    for (int i = 0; i < visibleSegments.size(); ++i)
    {
        const int segmentIndex = visibleStartIndex + i;
        const bool isFirst = segmentIndex == 0;
        const bool isLast = segmentIndex == mPathSegments.size() - 1;
        auto* segmentWidget =
            createSegmentWidget(visibleSegments.at(i), segmentIndex, isFirst, isLast);
        layout->insertWidget(insertIndex++, segmentWidget, 0, Qt::AlignLeft | Qt::AlignVCenter);

        if (!isLast)
        {
            auto* separator = makeSeparatorLabel();
            layout->insertWidget(insertIndex++, separator);
        }
    }

    refreshOverflowPopup();
}

void Breadcrumb::closeOverflowPopup()
{
    if (mOverflowPopup)
    {
        updateOverflowButtonStyle(false);
        mOverflowPopup->close();
        mOverflowPopup = nullptr;
    }
}

void Breadcrumb::updateOverflowButtonStyle(bool popupVisible)
{
    ui->bOverflow->setProperty("popupVisible", popupVisible);
    ui->bOverflow->style()->unpolish(ui->bOverflow);
    ui->bOverflow->style()->polish(ui->bOverflow);
    ui->bOverflow->update();
}

void Breadcrumb::refreshOverflowPopup()
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

void Breadcrumb::updateOverflowPopupContent()
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

    mOverflowPopup->setSegments(hiddenSegments, mOverflowSegmentsNavigable ? 0 : -1);

    positionOverflowPopup();
}

void Breadcrumb::positionOverflowPopup()
{
    if (!mOverflowPopup)
    {
        return;
    }

    // Horizontal: anchored to the overflow button (shifted left). Vertical: anchored to the
    // bottom of the whole path row so the popup clears it.
    const int globalX = ui->bOverflow->mapToGlobal(QPoint(POPUP_HORIZONTAL_OFFSET, 0)).x();
    const int globalY = mapToGlobal(QPoint(0, height())).y() + POPUP_VERTICAL_GAP;
    mOverflowPopup->move(globalX, globalY);
}

int Breadcrumb::calculateVisibleStartIndex()
{
    const int totalSegments = mPathSegments.size();
    if (totalSegments <= 1)
    {
        return 0;
    }

    const int minimumVisibleStartIndex = qMax(0, totalSegments - MAX_VISIBLE_DESTINATION_LEVELS);
    const int availableWidth = ui->wSegments->contentsRect().width();

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

int Breadcrumb::calculateRequiredWidthForVisibleStartIndex(int visibleStartIndex)
{
    auto* layout = qobject_cast<QHBoxLayout*>(ui->wSegments->layout());
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

void Breadcrumb::recalculateSegmentMetrics()
{
    mSegmentWidths.clear();
    mSegmentWidths.reserve(mPathSegments.size());

    for (int i = 0; i < mPathSegments.size(); ++i)
    {
        const bool isLast = i == mPathSegments.size() - 1;
        auto* segmentWidget = createSegmentWidget(mPathSegments.at(i), i, i == 0, isLast);
        segmentWidget->ensurePolished();
        mSegmentWidths.push_back(segmentWidget->sizeHint().width());
        delete segmentWidget;
    }

    auto* separator = makeSeparatorLabel();
    separator->ensurePolished();
    mSeparatorWidth = separator->sizeHint().width();
    delete separator;

    ui->bOverflow->ensurePolished();
    mOverflowWidth = ui->bOverflow->sizeHint().width();
}

void Breadcrumb::showOverflowPopup()
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
                emit overflowSegmentActivated(index);
                closeOverflowPopup();
            });

    mOverflowPopup = popup;
    updateOverflowButtonStyle(true);
    // Populate, size and position BEFORE showing: on Wayland an xdg_popup's position is fixed by
    // its positioner at map time (i.e. at show()), and a move() issued afterwards is an unreliable
    // reposition request that older compositors/Qt builds ignore, leaving the popup at a
    // compositor-chosen spot. Setting geometry first gives the positioner the correct anchor.
    updateOverflowPopupContent();
    popup->show();
    // Re-apply the position after showing as well: a top-level Qt::Popup may ignore a move()
    // issued before it is shown (notably on macOS). Cheap, no content rebuild.
    positionOverflowPopup();
}
