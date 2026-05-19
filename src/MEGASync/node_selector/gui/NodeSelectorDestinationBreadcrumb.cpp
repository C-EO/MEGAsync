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
    mPathSegments = segments;
    rebuildSegments();
    ui->bDestinationClear->setVisible(!mPathSegments.isEmpty());
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

QLabel* NodeSelectorDestinationBreadcrumb::makeSegmentLabel(const QString& text,
                                                            bool isFirst,
                                                            bool isLast)
{
    auto* label = new QLabel(ui->wDestinationSegments);
    label->setText(text);
    label->setProperty("font-size", QLatin1String("body-2"));
    label->setProperty((!isFirst && isLast) ? "bold" : "regular", true);
    return label;
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
    ui->cbAlwaysUploadToLocation->setVisible(mShouldShowDefaultUploadOption);
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
        auto* segmentLabel = makeSegmentLabel(visibleSegments.at(i), isFirst, isLast);
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
    popup->setSegments(hiddenSegments);

    connect(popup,
            &QObject::destroyed,
            this,
            [this]()
            {
                mOverflowPopup = nullptr;
            });

    mOverflowPopup = popup;
    popup->move(ui->wDestinationPath->mapToGlobal(QPoint(0, ui->wDestinationPath->height() + 4)));
    popup->show();
}
