#ifndef NODESELECTORDESTINATIONBREADCRUMB_H
#define NODESELECTORDESTINATIONBREADCRUMB_H

#include "NodeSelectorBreadcrumbSegment.h"

#include <QFrame>
#include <QPointer>
#include <QStringList>

class QLabel;
class NodeSelectorDestinationOverflowPopup;
class QResizeEvent;

namespace Ui
{
class NodeSelectorDestinationBreadcrumb;
}

class NodeSelectorDestinationBreadcrumb: public QFrame
{
    Q_OBJECT

public:
    enum class DisplayMode
    {
        DESTINATION,
        NAVIGATION
    };

    explicit NodeSelectorDestinationBreadcrumb(QWidget* parent = nullptr);
    ~NodeSelectorDestinationBreadcrumb() override;

    void setSegments(const QList<NodeSelectorBreadcrumbSegment>& segments, bool clickable = false);
    void setDisplayMode(DisplayMode mode);
    void setTitleText(const QString& text);
    void showDefaultUploadOption(bool show = true);
    void setDefaultUploadOption(bool value);
    bool getDefaultUploadOption() const;

protected:
    void resizeEvent(QResizeEvent* event) override;

signals:
    void clearRequested();
    void segmentActivated(int index);

private:
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
    QWidget* makeSegmentWidget(const QString& text,
                               bool isFirst,
                               bool isLast,
                               int segmentIndex,
                               bool applyInteractivity = true);
    QLabel* makeSeparatorLabel();
    void updateContentVisibility();
    int calculateVisibleStartIndex();
    int calculateRequiredWidthForVisibleStartIndex(int visibleStartIndex);

    Ui::NodeSelectorDestinationBreadcrumb* ui;
    QPointer<NodeSelectorDestinationOverflowPopup> mOverflowPopup;
    QList<NodeSelectorBreadcrumbSegment> mSegments;
    QStringList mPathSegments;
    DisplayMode mDisplayMode = DisplayMode::DESTINATION;
    bool mShouldShowDefaultUploadOption = false;
    bool mSegmentsClickable = false;
    int mVisibleStartIndex = 0;
    int mRenderedStartIndex = -1;
    QList<int> mSegmentWidths;
    int mSeparatorWidth = 0;
    int mOverflowWidth = 0;
};

#endif // NODESELECTORDESTINATIONBREADCRUMB_H
