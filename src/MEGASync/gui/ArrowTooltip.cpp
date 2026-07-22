#include "ArrowTooltip.h"

#include "TokenParserWidgetManager.h"
#include "ui_ArrowTooltip.h"

#include <QPainter>
#include <QPainterPath>

ArrowTooltip::ArrowTooltip(QWidget* parent):
    QFrame(parent, Qt::ToolTip | Qt::FramelessWindowHint),
    ui(new Ui::ArrowTooltip)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_StyledBackground, true);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAutoFillBackground(false);
}

ArrowTooltip::~ArrowTooltip()
{
    delete ui;
}

void ArrowTooltip::setText(const QString& text)
{
    ui->lText->setText(text);
    ui->lText->adjustSize();
    adjustSize();
    TokenParserWidgetManager::instance()->applyCurrentTheme(this);
}

void ArrowTooltip::setFontSize(const QString& fontSize, bool semibold)
{
    ui->lText->setProperty("font-size", fontSize);
    // [semibold]/[regular] are presence-based QSS selectors, so the unused weight must be cleared
    // (QVariant()) rather than set to false.
    ui->lText->setProperty("semibold", semibold ? QVariant(true) : QVariant());
    ui->lText->setProperty("regular", semibold ? QVariant() : QVariant(true));

    ui->lText->adjustSize();
    adjustSize();
    TokenParserWidgetManager::instance()->applyCurrentTheme(this);
}

void ArrowTooltip::setArrow(Arrow arrow, int arrowTipX)
{
    mArrow = arrow;
    mArrowTipX = arrowTipX;

    // Reserve room for the arrow on its side so it doesn't overlap the content.
    const int top = arrow == Arrow::Up ? ARROW_HEIGHT : 0;
    const int bottom = arrow == Arrow::Down ? ARROW_HEIGHT : 0;
    ui->rootLayout->setContentsMargins(0, top, 0, bottom);

    adjustSize();
    update();
}

void ArrowTooltip::paintEvent(QPaintEvent* event)
{
    QFrame::paintEvent(event);

    if (mArrow == Arrow::None)
    {
        return;
    }

    const int tipX = mArrowTipX >= 0 ? mArrowTipX : width() / 2;
    const int half = ARROW_WIDTH / 2;

    QPainterPath path;
    if (mArrow == Arrow::Up)
    {
        path.moveTo(tipX, 0);
        path.lineTo(tipX - half, ARROW_HEIGHT);
        path.lineTo(tipX + half, ARROW_HEIGHT);
    }
    else
    {
        const int baseY = height() - ARROW_HEIGHT;
        path.moveTo(tipX, height());
        path.lineTo(tipX - half, baseY);
        path.lineTo(tipX + half, baseY);
    }
    path.closeSubpath();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.fillPath(
        path,
        TokenParserWidgetManager::instance()->getColor(QLatin1String("background-inverse")));
}
