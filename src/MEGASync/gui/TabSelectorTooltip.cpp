#include "TabSelectorTooltip.h"

#include "TokenParserWidgetManager.h"
#include "ui_TabSelectorTooltip.h"

TabSelectorTooltip::TabSelectorTooltip(QWidget* parent):
    QFrame(parent, Qt::ToolTip | Qt::FramelessWindowHint),
    ui(new Ui::TabSelectorTooltip)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_StyledBackground, true);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAutoFillBackground(false);
}

TabSelectorTooltip::~TabSelectorTooltip()
{
    delete ui;
}

void TabSelectorTooltip::setText(const QString& text)
{
    ui->lText->setText(text);
    ui->lText->adjustSize();
    adjustSize();
    TokenParserWidgetManager::instance()->applyCurrentTheme(this);
}
