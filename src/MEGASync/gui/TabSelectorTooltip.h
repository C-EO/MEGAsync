#ifndef TABSELECTORTOOLTIP_H
#define TABSELECTORTOOLTIP_H

#include <QFrame>

namespace Ui
{
class TabSelectorTooltip;
}

class TabSelectorTooltip: public QFrame
{
    Q_OBJECT

public:
    explicit TabSelectorTooltip(QWidget* parent = nullptr);
    ~TabSelectorTooltip() override;

    void setText(const QString& text);

private:
    Ui::TabSelectorTooltip* ui;
};

#endif // TABSELECTORTOOLTIP_H
