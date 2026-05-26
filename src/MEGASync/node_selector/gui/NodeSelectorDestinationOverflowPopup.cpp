#include "NodeSelectorDestinationOverflowPopup.h"

#include "TokenParserWidgetManager.h"
#include "ui_NodeSelectorDestinationOverflowPopup.h"

#include <QLabel>
#include <QLayoutItem>
#include <QToolButton>
#include <QVBoxLayout>

NodeSelectorDestinationOverflowPopup::NodeSelectorDestinationOverflowPopup(QWidget* parent):
    QFrame(parent, Qt::Popup | Qt::FramelessWindowHint),
    ui(new Ui::NodeSelectorDestinationOverflowPopup)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_StyledBackground, true);
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);
}

NodeSelectorDestinationOverflowPopup::~NodeSelectorDestinationOverflowPopup()
{
    delete ui;
}

void NodeSelectorDestinationOverflowPopup::setSegments(const QStringList& segments, int indexOffset)
{
    clearLabels();

    auto* layout = qobject_cast<QVBoxLayout*>(ui->scrollContent->layout());
    if (!layout)
    {
        return;
    }

    static constexpr int MAX_VISIBLE_ROWS = 5;
    static constexpr int ROW_HEIGHT = 20;

    for (int i = 0; i < segments.size(); ++i)
    {
        const auto& segment = segments.at(i);

        if (indexOffset >= 0)
        {
            auto* button = new QToolButton(ui->scrollContent);
            button->setText(segment);
            button->setToolButtonStyle(Qt::ToolButtonTextOnly);
            button->setAutoRaise(true);
            button->setCursor(Qt::PointingHandCursor);
            button->setMinimumHeight(ROW_HEIGHT);
            button->setProperty("font-size", QLatin1String("body-2"));
            button->setProperty("regular", true);

            connect(button,
                    &QToolButton::clicked,
                    this,
                    [this, segmentIndex = indexOffset + i]()
                    {
                        emit segmentActivated(segmentIndex);
                    });

            layout->addWidget(button);
        }
        else
        {
            auto* label = new QLabel(ui->scrollContent);
            label->setText(segment);
            label->setMinimumHeight(ROW_HEIGHT);
            // No eliding: the label keeps its natural width so the popup can resize-to-contents.
            label->setProperty("font-size", QLatin1String("body-2"));
            label->setProperty("regular", true);
            layout->addWidget(label);
        }
    }

    // Force layout/sizeHint refresh so the QScrollArea picks the natural content width.
    ui->scrollContent->adjustSize();

    // Compute the exact height: min(MAX_VISIBLE_ROWS, segments.size()) rows + margins/spacing.
    // The scrollArea is pinned to this height so it shrinks when there are few segments and
    // shows a vertical scrollbar when there are more than MAX_VISIBLE_ROWS.
    const int visibleRows = qMin(MAX_VISIBLE_ROWS, segments.size());
    const bool needsScrollbar = segments.size() > MAX_VISIBLE_ROWS;
    const auto margins = layout->contentsMargins();
    const int spacings = qMax(0, visibleRows - 1) * layout->spacing();
    const int targetHeight = margins.top() + margins.bottom() + visibleRows * ROW_HEIGHT + spacings;
    static const int POPUP_BORDER_WIDTH = 1;
    const int framedTargetHeight = targetHeight + (needsScrollbar ? 0 : 2 * POPUP_BORDER_WIDTH);

    ui->scrollArea->setVerticalScrollBarPolicy(needsScrollbar ? Qt::ScrollBarAsNeeded :
                                                                Qt::ScrollBarAlwaysOff);
    ui->scrollArea->setFixedHeight(framedTargetHeight);
    adjustSize();
    setFixedHeight(framedTargetHeight);

    TokenParserWidgetManager::instance()->applyCurrentTheme(this);
}

void NodeSelectorDestinationOverflowPopup::clearLabels()
{
    auto* layout = qobject_cast<QVBoxLayout*>(ui->scrollContent->layout());
    if (!layout)
    {
        return;
    }

    while (auto* item = layout->takeAt(0))
    {
        if (auto* w = item->widget())
        {
            w->deleteLater();
        }
        delete item;
    }
}
