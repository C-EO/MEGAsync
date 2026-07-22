#ifndef ARROWTOOLTIP_H
#define ARROWTOOLTIP_H

#include <QFrame>

namespace Ui
{
class ArrowTooltip;
}

class ArrowTooltip: public QFrame
{
    Q_OBJECT

public:
    enum class Arrow
    {
        None,
        Up,
        Down
    };

    explicit ArrowTooltip(QWidget* parent = nullptr);
    ~ArrowTooltip() override;

    void setText(const QString& text);
    // Token-based text style. fontSize is a font-size token ("caption", "body-2", ...);
    // semibold=false renders Regular (400) weight.
    void setFontSize(const QString& fontSize, bool semibold = true);
    // Pointer arrow on the given side. arrowTipX is the tip X in tooltip-local coords
    // (-1 = centered). Arrow::None (default) keeps the tab look (no arrow).
    void setArrow(Arrow arrow, int arrowTipX = -1);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    static constexpr int ARROW_WIDTH = 12;
    static constexpr int ARROW_HEIGHT = 6;

    Ui::ArrowTooltip* ui;
    Arrow mArrow = Arrow::None;
    int mArrowTipX = -1;
};

#endif // ARROWTOOLTIP_H
