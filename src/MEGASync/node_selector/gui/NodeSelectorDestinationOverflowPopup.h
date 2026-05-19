#ifndef NODESELECTORDESTINATIONOVERFLOWPOPUP_H
#define NODESELECTORDESTINATIONOVERFLOWPOPUP_H

#include <QFrame>
#include <QStringList>

namespace Ui
{
class NodeSelectorDestinationOverflowPopup;
}

class NodeSelectorDestinationOverflowPopup: public QFrame
{
    Q_OBJECT

public:
    explicit NodeSelectorDestinationOverflowPopup(QWidget* parent = nullptr);
    ~NodeSelectorDestinationOverflowPopup() override;

    void setSegments(const QStringList& segments);

private:
    void clearLabels();

    Ui::NodeSelectorDestinationOverflowPopup* ui;
};

#endif // NODESELECTORDESTINATIONOVERFLOWPOPUP_H
