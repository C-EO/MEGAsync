#ifndef BREADCRUMB_H
#define BREADCRUMB_H

#include "NodeSelectorBreadcrumbSegment.h"

#include <QFrame>
#include <QPointer>
#include <QStringList>

#include <functional>

class QLabel;
class NodeSelectorDestinationOverflowPopup;
class QResizeEvent;

namespace Ui
{
class Breadcrumb;
}

// Generic breadcrumb engine: lays out segment widgets with separators, collapses overflow into a
// "…" popup and handles metrics/resize. It knows nothing about what a segment is or whether it is
// interactive: the owner supplies a factory that builds each segment widget.
class Breadcrumb: public QFrame
{
    Q_OBJECT

public:
    // Builds the widget shown for one path segment.
    using SegmentFactory =
        std::function<QWidget*(const QString& text, int index, bool isFirst, bool isLast)>;

    explicit Breadcrumb(QWidget* parent = nullptr);
    ~Breadcrumb() override;

    void setSegmentFactory(SegmentFactory factory);
    void setSegments(const QList<NodeSelectorBreadcrumbSegment>& segments,
                     bool overflowSegmentsNavigable = false);

signals:
    // A hidden ancestor segment was chosen from the overflow popup.
    void overflowSegmentActivated(int index);
    // A shown segment's node was renamed, so its label is now stale.
    void refreshNeeded();

public slots:
    void onNodesRenamed(const QList<mega::MegaHandle>& handles);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    bool containsAnyHandle(const QList<mega::MegaHandle>& handles) const;
    void closeOverflowPopup();
    void showOverflowPopup();
    void updateOverflowButtonStyle(bool popupVisible);
    void rebuildSegments(bool force = true);
    void refreshOverflowPopup();
    void updateOverflowPopupContent();
    void resolveSegmentNames();
    QString resolveSegmentText(const NodeSelectorBreadcrumbSegment& segment) const;
    void recalculateSegmentMetrics();
    void clearSegmentWidgets();
    QWidget* createSegmentWidget(const QString& text, int index, bool isFirst, bool isLast);
    QLabel* makeSeparatorLabel();
    int calculateVisibleStartIndex();
    int calculateRequiredWidthForVisibleStartIndex(int visibleStartIndex);

    Ui::Breadcrumb* ui;
    QPointer<NodeSelectorDestinationOverflowPopup> mOverflowPopup;
    QList<NodeSelectorBreadcrumbSegment> mSegments;
    QStringList mPathSegments;
    SegmentFactory mSegmentFactory;
    bool mOverflowSegmentsNavigable = false;
    int mVisibleStartIndex = 0;
    int mRenderedStartIndex = -1;
    QList<int> mSegmentWidths;
    int mSeparatorWidth = 0;
    int mOverflowWidth = 0;
};

#endif // BREADCRUMB_H
