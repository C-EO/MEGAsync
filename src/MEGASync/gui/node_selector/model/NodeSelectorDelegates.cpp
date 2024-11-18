#include "NodeSelectorDelegates.h"

#include "NodeSelectorModel.h"
#include "NodeSelectorTreeView.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QBitmap>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QToolTip>

NodeSelectorDelegate::NodeSelectorDelegate(QObject* parent):
    QStyledItemDelegate(parent)
{}

void NodeSelectorDelegate::setPaintDevice(QPainter* painter) const
{
    // First time the row is painted, we get the main paint device
    // in order to compare with other paint devices, as the use by the dragging action
    if (!mMainDevice)
    {
        mMainDevice = painter->device();
    }
}

bool NodeSelectorDelegate::isPaintingDrag(QPainter* painter) const
{
    auto view = dynamic_cast<NodeSelectorTreeView*>(parent());
    if (view)
    {
        return view->state() == NodeSelectorTreeView::DraggingState &&
               painter->device() != mMainDevice;
    }

    return false;
}

////////////////////////////////////////////////////////////////////////7

const int IconDelegate::ICON_HEIGHT = 18;

IconDelegate::IconDelegate(QObject* parent):
    NodeSelectorDelegate(parent)
{
}

IconDelegate::~IconDelegate()
{
}


void IconDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    NodeSelectorDelegate::setPaintDevice(painter);

    if (!isPaintingDrag(painter))
    {
        NodeSelectorDelegate::paint(painter, option, index);
        QStyleOptionViewItem opt(option);
        opt.decorationAlignment = index.data(Qt::TextAlignmentRole).value<Qt::Alignment>();

        QIcon icon;
        QIcon::Mode iconMode = QIcon::Normal;
        if (index.data(Qt::DecorationRole).canConvert<QIcon>())
        {
            icon = index.data(Qt::DecorationRole).value<QIcon>();

            if (option.state.testFlag(QStyle::State_Selected))
            {
                iconMode = QIcon::Selected;
            }
        }
        else if (index.data(Qt::DecorationRole).canConvert<QPixmap>())
        {
            icon.addPixmap(index.data(Qt::DecorationRole).value<QPixmap>());
        }

        QRect iconRect(QPoint(option.rect.topLeft()), QSize(ICON_HEIGHT, ICON_HEIGHT));
        iconRect.moveCenter(option.rect.center());
        icon.paint(painter, iconRect, Qt::AlignVCenter | Qt::AlignHCenter, iconMode);
    }
}

void IconDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    option->icon = QIcon();
}


const int NodeRowDelegate::MARGIN = 7;
const int NodeRowDelegate::ICON_MARGIN = 37;
const int NodeRowDelegate::DIFF_WITH_STD_ICON = 5;

NodeRowDelegate::NodeRowDelegate(QObject* parent):
    NodeSelectorDelegate(parent)
{

}

void NodeRowDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt(option);
    int indentValue = index.data(toInt(NodeRowDelegateRoles::INDENT_ROLE)).toInt();
    opt.rect.adjust(indentValue, 0, 0, 0);
    QVariant small_icon = index.data(toInt(NodeRowDelegateRoles::SMALL_ICON_ROLE));
    if(small_icon.isValid() && small_icon.toBool())
        opt.decorationSize = QSize(opt.decorationSize.width() - DIFF_WITH_STD_ICON, opt.decorationSize.height() - DIFF_WITH_STD_ICON);

    NodeSelectorDelegate::setPaintDevice(painter);
    if (isPaintingDrag(painter))
    {
        QPainterPath selectedPath;
        selectedPath
            .addRoundedRect(opt.rect.x(), opt.rect.y(), opt.rect.width(), opt.rect.height(), 4, 4);
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(option.palette.highlight());
        painter->drawPath(selectedPath);
        painter->restore();

        opt.state.setFlag(QStyle::State_Selected, false);
    }

    NodeSelectorDelegate::paint(painter, opt, index);
}

bool NodeRowDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if ( !event || !view || !index.isValid())
    {
        return false;
    }

    if (event->type() == QEvent::ToolTip)
    {
        QRect rect = view->visualRect(index);
        QString tooltipText = index.data(Qt::DisplayRole).toString();
        QFontMetrics fm = option.fontMetrics;

        int margin = MARGIN;
        if(index.column() == NodeSelectorModel::NODE)
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

    return QStyledItemDelegate::helpEvent(event, view, option, index );
}

void NodeRowDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    if(!index.flags().testFlag(Qt::ItemIsEnabled))
    {
        option->state &= ~QStyle::State_Enabled;
    }
}

DateColumnDelegate::DateColumnDelegate(QObject* parent):
    NodeSelectorDelegate(parent)
{
}

void DateColumnDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    NodeSelectorDelegate::setPaintDevice(painter);

    if (!isPaintingDrag(painter))
    {
        NodeSelectorDelegate::paint(painter, option, index);
        painter->save();
        QPalette::ColorGroup cg = QPalette::Normal;
        if (!index.flags().testFlag(Qt::ItemIsEnabled))
        {
            cg = QPalette::Disabled;
        }
        if (option.state & QStyle::State_Selected)
        {
            painter->setPen(option.palette.color(cg, QPalette::HighlightedText));
        }
        else
        {
            painter->setPen(option.palette.color(cg, QPalette::Text));
        }
        QRect rect = option.rect;
        rect.adjust(10, 0, -5, 0);
        QString elideText = option.fontMetrics.elidedText(index.data(Qt::DisplayRole).toString(),
                                                          Qt::ElideMiddle,
                                                          rect.width());
        painter->drawText(rect, Qt::AlignVCenter, elideText);
        painter->restore();
    }
}

bool DateColumnDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (!event || !view || !index.isValid())
    {
        return false;
    }

    if (event->type() == QEvent::ToolTip)
    {
        QRect rect = view->visualRect(index);
        rect.adjust(10, 0, -5, 0);
        QString tooltipText = index.data(Qt::DisplayRole).toString();
        QFontMetrics fm = option.fontMetrics;

        if (rect.width() < (fm.horizontalAdvance(tooltipText)))
        {
                QToolTip::showText(event->globalPos(), tooltipText);
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

void DateColumnDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    option->text = QString::fromUtf8("");
}
