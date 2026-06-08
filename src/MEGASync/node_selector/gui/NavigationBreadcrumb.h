#ifndef NAVIGATIONBREADCRUMB_H
#define NAVIGATIONBREADCRUMB_H

#include "NodeSelectorBreadcrumbSegment.h"

#include <QFrame>
#include <QList>

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

    void setSegments(const QList<NodeSelectorBreadcrumbSegment>& segments,
                     bool currentRootReadOnly = false);

signals:
    void segmentActivated(int index);
    void lastSegmentMenuRequested(const QPoint& globalPos);
    void refreshNeeded();

public slots:
    void onNodesRenamed(const QList<mega::MegaHandle>& handles);

private:
    Ui::NavigationBreadcrumb* ui;
    bool mCurrentRootReadOnly = false;
};

#endif // NAVIGATIONBREADCRUMB_H
