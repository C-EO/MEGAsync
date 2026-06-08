#include "NodeSelectorDelegates.h"

#include "MegaDelegateHoverManager.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorTreeView.h"
#include "TokenParserWidgetManager.h"

#include <QAbstractItemView>
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QTextDocument>
#include <QToolTip>

NodeSelectorDelegate::NodeSelectorDelegate(QObject* parent):
    QStyledItemDelegate(parent)
{}

void NodeSelectorDelegate::paint(QPainter* painter,
                                 const QStyleOptionViewItem& option,
                                 const QModelIndex& index) const
{
    // The background for each row is painted in NodeSeletorTreeView::drawRows

    QStyleOptionViewItem auxOpt(option);

    if (!index.data(toInt(NodeSelectorModelRoles::EXTRA_ROW_ROLE)).toBool())
    {
        auto pen(painter->pen());
        pen.setWidth(1);

        // Text color
        const auto isTakenDown =
            index.data(toInt(NodeSelectorModelRoles::IS_TAKEN_DOWN_ROLE)).toBool();
        const auto textColor = textColorForIndex(index, isTakenDown);
        auxOpt.palette.setBrush(QPalette::ColorRole::Text, textColor);

        if (!isTakenDown && index.flags().testFlag(Qt::ItemIsEnabled))
        {
            auxOpt.palette.setBrush(QPalette::ColorRole::HighlightedText, textColor);
        }

        // Separator
        {
            painter->save();
            pen.setColor(
                TokenParserWidgetManager::instance()->getColor(QLatin1String("border-subtle")));
            painter->setPen(pen);

            int y = option.rect.bottomLeft().y();
            int leftX = index.column() == 0 ? 4 : option.rect.x();
            int rightX = option.rect.x();
            rightX += index.column() == index.model()->columnCount() - 1 ?
                          (option.rect.width() - 4) :
                          option.rect.width();

            painter->drawLine(QLine(QPoint(leftX, y), QPoint(rightX, y)));
            painter->restore();
        }

        // Adjust the content to align it with the header
#ifdef Q_OS_MACOS
        auxOpt.rect.adjust(0, 0, -5, 0);
#else
        auxOpt.rect.adjust(3, 0, -5, 0);
#endif

        adjustContentRect(&auxOpt, index);
    }

    auxOpt.state.setFlag(QStyle::State_MouseOver, false);
    auxOpt.state.setFlag(QStyle::State_Selected, false);
    auxOpt.state.setFlag(QStyle::State_HasFocus, false);

    QStyledItemDelegate::paint(painter, auxOpt, index);
}

bool NodeSelectorDelegate::isHoverStateSet(const QModelIndex& index)
{
    if (!mLastHoverRow.isValid())
    {
        return false;
    }

    if (mLastHoverRow.data(toInt(NodeSelectorModelRoles::EXTRA_ROW_ROLE)).toBool())
    {
        return false;
    }

    return (mLastHoverRow.parent() == index.parent() && mLastHoverRow.row() == index.row());
}

bool NodeSelectorDelegate::event(QEvent* event)
{
    if (auto hoverEvent = dynamic_cast<MegaDelegateHoverEvent*>(event))
    {
        switch (static_cast<int>(hoverEvent->type()))
        {
            case MegaDelegateHoverEvent::Enter:
            {
                mLastHoverRow = hoverEvent->index();
                break;
            }
            case MegaDelegateHoverEvent::Leave:
            {
                mLastHoverRow = QModelIndex();
                break;
            }
            default:
            {
                break;
            }
        }
    }

    return QStyledItemDelegate::event(event);
}

QColor NodeSelectorDelegate::textColorForIndex(const QModelIndex& index, bool isTakenDown) const
{
    if (isTakenDown)
    {
        auto textColor =
            TokenParserWidgetManager::instance()->getColor(QLatin1String("text-error"));

        if (!index.flags().testFlag(Qt::ItemIsEnabled))
        {
            static constexpr double ALPHA_CORRECTION_FOR_ERROR_DISABLED = 0.5;
            textColor.setAlphaF(ALPHA_CORRECTION_FOR_ERROR_DISABLED);
        }

        return textColor;
    }
    if (!index.flags().testFlag(Qt::ItemIsEnabled))
    {
        return TokenParserWidgetManager::instance()->getColor(QLatin1String("text-disabled"));
    }
    return TokenParserWidgetManager::instance()->getColor(QLatin1String("text-primary"));
}

const int NodeRowDelegate::MARGIN = 7;
const int NodeRowDelegate::ICON_MARGIN = 37;
const int NodeRowDelegate::DIFF_WITH_STD_ICON = 5;
const int NodeRowDelegate::ROW_HEIGHT = 40;

NodeRowDelegate::NodeRowDelegate(QObject* parent):
    NodeSelectorDelegate(parent)
{}

void NodeRowDelegate::paint(QPainter* painter,
                            const QStyleOptionViewItem& option,
                            const QModelIndex& index) const
{
    QStyleOptionViewItem opt(option);

    if (index.column() == NodeSelectorModel::Column::IS_EXPORTED)
    {
        opt.displayAlignment = Qt::AlignCenter;
        opt.decorationAlignment = Qt::AlignCenter;
    }
    else
    {
        opt.displayAlignment = Qt::AlignVCenter | Qt::AlignLeft;
        opt.decorationAlignment = Qt::AlignVCenter | Qt::AlignLeft;
    }
    opt.decorationSize = index.data(toInt(NodeSelectorModelRoles::ICON_SIZE_ROLE)).toSize();

    NodeSelectorDelegate::paint(painter, opt, index);
}

QPixmap NodeRowDelegate::paintForDrag(const QModelIndex& index, QAbstractItemView* view) const
{
    if (!view || !index.isValid())
    {
        return {};
    }

    QRect rect = view->visualRect(index);
    if (!rect.isValid() || rect.isEmpty())
    {
        QStyleOptionViewItem fallbackOption;
        fallbackOption.initFrom(view);
        fallbackOption.widget = view;
        initStyleOption(&fallbackOption, index);

        const auto hint = sizeHint(fallbackOption, index);
        if (!hint.isValid() || hint.isEmpty())
        {
            return {};
        }

        rect = QRect(QPoint(0, 0), hint);
    }

    // Limit the width to 400px max
    rect.setWidth(std::max(1, std::min(rect.width(), 400)));
    rect.setHeight(std::max(1, rect.height()));

    QPixmap pixmap(rect.size());
    pixmap.fill(Qt::transparent);
    if (pixmap.isNull())
    {
        return {};
    }

    QStyleOptionViewItem option;
    option.initFrom(view);
    option.widget = view;
    initStyleOption(&option, index);
    option.rect = QRect(0, 0, rect.width(), rect.height());
    option.decorationPosition = QStyleOptionViewItem::Left;
    option.decorationAlignment = Qt::AlignCenter;
    option.displayAlignment = Qt::AlignVCenter | Qt::AlignLeft;
    option.textElideMode = view->textElideMode();
    option.showDecorationSelected =
        view->style()->styleHint(QStyle::SH_ItemView_ShowDecorationSelected, nullptr, view);

    QPainter painter(&pixmap);
    if (!painter.isActive())
    {
        return {};
    }

    QPainterPath path;
    auto backgroundRect(option.rect);
    backgroundRect.setRight(option.rect.right() - 10);
    backgroundRect.setTop(option.rect.top() + 3);
    backgroundRect.setBottom(option.rect.bottom() - 5);
    path.addRoundedRect(backgroundRect, 4, 4);
    painter.fillPath(path,
                     TokenParserWidgetManager::instance()->getColor(QLatin1String("surface-2")));

    paint(&painter, option, index);
    painter.end();

    return pixmap;
}

bool NodeRowDelegate::helpEvent(QHelpEvent* event,
                                QAbstractItemView* view,
                                const QStyleOptionViewItem& option,
                                const QModelIndex& index)
{
    if (!event || !view || !index.isValid())
    {
        return false;
    }

    if (event->type() == QEvent::ToolTip)
    {
        if (index.column() == NodeSelectorModel::Column::IS_EXPORTED)
        {
            QToolTip::hideText();
            return true;
        }

        const auto rect = view->visualRect(index);
        const auto tooltipText = index.data(Qt::DisplayRole).toString();
        QFontMetrics fm = option.fontMetrics;

        auto margin = MARGIN;
        if (index.column() == NodeSelectorModel::Column::NODE)
        {
            margin = ICON_MARGIN;
        }
        if (rect.width() < (fm.horizontalAdvance(tooltipText) + margin))
        {
            QToolTip::showText(event->globalPos(), tooltipText.toHtmlEscaped());
            return true;
        }
        if (!QStyledItemDelegate::helpEvent(event, view, option, index))
        {
            QToolTip::hideText();
        }
        return true;
    }

    return QStyledItemDelegate::helpEvent(event, view, option, index);
}

QSize NodeRowDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto size = NodeSelectorDelegate::sizeHint(option, index);
    size.setHeight(ROW_HEIGHT);
    return size;
}

void NodeRowDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const
{
    QStyledItemDelegate::initStyleOption(option, index);

    if (!index.flags().testFlag(Qt::ItemIsEnabled))
    {
        option->state &= ~QStyle::State_Enabled;
    }
}

NodeLabelDelegate::NodeLabelDelegate(bool showLabelText, QObject* parent):
    NodeSelectorDelegate(parent),
    mShowLabelText(showLabelText)
{}

void NodeLabelDelegate::paint(QPainter* painter,
                              const QStyleOptionViewItem& option,
                              const QModelIndex& index) const
{
    static constexpr int LABEL_DOT_LEFT_MARGIN = 12;
    static constexpr qreal LABEL_DOT_RADIUS = 4.0;

    QStyleOptionViewItem opt(option);
    opt.displayAlignment = Qt::AlignVCenter | Qt::AlignLeft;
    opt.decorationAlignment = Qt::AlignVCenter | Qt::AlignLeft;

    NodeSelectorDelegate::paint(painter, opt, index);

    const auto labelColor =
        index.data(toInt(NodeSelectorModelRoles::LABEL_COLOR_ROLE)).value<QColor>();
    if (labelColor.isValid())
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(labelColor);
        painter->drawEllipse(QPointF(option.rect.x() + LABEL_DOT_LEFT_MARGIN + LABEL_DOT_RADIUS,
                                     option.rect.center().y()),
                             LABEL_DOT_RADIUS,
                             LABEL_DOT_RADIUS);
        painter->restore();
    }
}

bool NodeLabelDelegate::helpEvent(QHelpEvent* event,
                                  QAbstractItemView* view,
                                  const QStyleOptionViewItem& option,
                                  const QModelIndex& index)
{
    if (!event || !view || !index.isValid())
    {
        return false;
    }

    if (event->type() == QEvent::ToolTip)
    {
        QToolTip::hideText();
        return true;
    }

    return QStyledItemDelegate::helpEvent(event, view, option, index);
}

QSize NodeLabelDelegate::sizeHint(const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const
{
    auto size = NodeSelectorDelegate::sizeHint(option, index);
    size.setHeight(NodeRowDelegate::ROW_HEIGHT);
    return size;
}

void NodeLabelDelegate::initStyleOption(QStyleOptionViewItem* option,
                                        const QModelIndex& index) const
{
    QStyledItemDelegate::initStyleOption(option, index);

    option->icon = QIcon();

    if (!mShowLabelText)
    {
        option->text.clear();
    }

    if (!index.flags().testFlag(Qt::ItemIsEnabled))
    {
        option->state &= ~QStyle::State_Enabled;
    }
}

void NodeLabelDelegate::adjustContentRect(QStyleOptionViewItem* option, const QModelIndex&) const
{
    if (mShowLabelText)
    {
        static constexpr int LABEL_TEXT_LEFT_MARGIN = 24;
        option->rect.adjust(LABEL_TEXT_LEFT_MARGIN, 0, 0, 0);
    }
}

NodeSearchRowDelegate::NodeSearchRowDelegate(QObject* parent):
    NodeRowDelegate(parent)
{}

void NodeSearchRowDelegate::setSearchText(const QString& text)
{
    if (mSearchText != text)
    {
        mSearchText = text;
        mDocumentCache.clear();
    }
}

void NodeSearchRowDelegate::paint(QPainter* painter,
                                  const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const
{
    const bool isNodeColumn = index.column() == NodeSelectorModel::Column::NODE;
    const QString display = index.data(Qt::DisplayRole).toString();
    const bool shouldHighlight = isNodeColumn && !mSearchText.isEmpty() &&
                                 display.contains(mSearchText, Qt::CaseInsensitive);

    if (!shouldHighlight)
    {
        NodeRowDelegate::paint(painter, option, index);
        return;
    }

    // Paint the row, without the text
    mSuppressText = true;
    NodeRowDelegate::paint(painter, option, index);
    mSuppressText = false;

    QStyleOptionViewItem optForRect(option);
    NodeRowDelegate::initStyleOption(&optForRect, index);
    const QWidget* widget = option.widget;
    const QStyle* style = widget ? widget->style() : QApplication::style();
    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &optForRect, widget);
    if (!textRect.isValid() || textRect.isEmpty())
    {
        return;
    }

    // Building the highlighted HTML and laying out a QTextDocument is expensive, and paint() runs
    // for every visible matching row on each repaint. Cache the laid-out document per
    // (display text, search term, available width). The cache is cleared when the search text
    // changes and bounded to avoid unbounded growth while scrolling a long result list.
    const QString cacheKey = display + QLatin1Char('\n') + mSearchText + QLatin1Char('\n') +
                             QString::number(textRect.width());
    auto doc = mDocumentCache.value(cacheKey);
    if (!doc)
    {
        static constexpr int MAX_CACHED_DOCUMENTS = 256;
        if (mDocumentCache.size() >= MAX_CACHED_DOCUMENTS)
        {
            mDocumentCache.clear();
        }

        QFontMetrics fm(option.font);
        QString shown = display;
        if (fm.horizontalAdvance(display) > textRect.width())
        {
            shown = fm.elidedText(display, option.textElideMode, textRect.width());
        }

        doc = std::make_shared<QTextDocument>();
        doc->setDefaultFont(option.font);
        doc->setDocumentMargin(0);
        doc->setHtml(buildHighlightedHtml(shown, mSearchText));
        doc->setTextWidth(textRect.width());
        mDocumentCache.insert(cacheKey, doc);
    }

    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.palette = optForRect.palette;
    const auto isTakenDown = index.data(toInt(NodeSelectorModelRoles::IS_TAKEN_DOWN_ROLE)).toBool();
    ctx.palette.setColor(QPalette::Text, textColorForIndex(index, isTakenDown));
    ctx.clip = QRectF(0, 0, textRect.width(), textRect.height());

    painter->save();
    const qreal yOffset = (textRect.height() - doc->size().height()) / 2.0;
    painter->translate(textRect.topLeft() + QPointF(0, yOffset));
    doc->documentLayout()->draw(painter, ctx);
    painter->restore();
}

void NodeSearchRowDelegate::initStyleOption(QStyleOptionViewItem* option,
                                            const QModelIndex& index) const
{
    NodeRowDelegate::initStyleOption(option, index);
    if (mSuppressText)
    {
        option->text.clear();
    }
}

QString NodeSearchRowDelegate::buildHighlightedHtml(const QString& display, const QString& search)
{
    QString result;
    int cursor = 0;
    while (cursor < display.size())
    {
        const int matchStart = display.indexOf(search, cursor, Qt::CaseInsensitive);
        if (matchStart < 0)
        {
            result += display.mid(cursor).toHtmlEscaped();
            break;
        }

        const QString beforeMatch = display.mid(cursor, matchStart - cursor);
        const QString match = display.mid(matchStart, search.size());
        result += beforeMatch.toHtmlEscaped();
        result += QStringLiteral("<b>") + match.toHtmlEscaped() + QStringLiteral("</b>");

        cursor = matchStart + search.size();
    }
    return result;
}
