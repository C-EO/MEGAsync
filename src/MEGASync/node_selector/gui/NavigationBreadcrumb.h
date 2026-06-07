#ifndef NAVIGATIONBREADCRUMB_H
#define NAVIGATIONBREADCRUMB_H

#include "NodeSelectorBreadcrumbSegment.h"

#include <QFrame>

namespace Ui
{
class NavigationBreadcrumb;
}

class NavigationBreadcrumb: public QFrame
{
    Q_OBJECT

public:
    explicit NavigationBreadcrumb(QWidget* parent = nullptr);
    ~NavigationBreadcrumb() override;

    void setSegments(const QList<NodeSelectorBreadcrumbSegment>& segments);

signals:
    void segmentActivated(int index);
    void lastSegmentMenuRequested(const QPoint& globalPos);

private:
    Ui::NavigationBreadcrumb* ui;
};

#endif // NAVIGATIONBREADCRUMB_H
