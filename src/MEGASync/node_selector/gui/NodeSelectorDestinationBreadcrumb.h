#ifndef NODESELECTORDESTINATIONBREADCRUMB_H
#define NODESELECTORDESTINATIONBREADCRUMB_H

#include <QFrame>
#include <QPointer>
#include <QStringList>

class QLabel;
class NodeSelectorDestinationOverflowPopup;

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

    void setPathSegments(const QStringList& segments);
    void setNavigationSegments(const QStringList& segments);
    void setDisplayMode(DisplayMode mode);
    void setTitleText(const QString& text);
    void showDefaultUploadOption(bool show = true);
    void setDefaultUploadOption(bool value);
    bool getDefaultUploadOption() const;

signals:
    void clearRequested();
    void segmentActivated(int index);

private:
    void closeOverflowPopup();
    void showOverflowPopup();
    void rebuildSegments();
    void clearSegmentWidgets();
    QWidget* makeSegmentWidget(const QString& text, bool isFirst, bool isLast, int segmentIndex);
    QLabel* makeSeparatorLabel();
    void updateContentVisibility();

    Ui::NodeSelectorDestinationBreadcrumb* ui;
    QPointer<NodeSelectorDestinationOverflowPopup> mOverflowPopup;
    QStringList mPathSegments;
    DisplayMode mDisplayMode = DisplayMode::DESTINATION;
    bool mShouldShowDefaultUploadOption = false;
    bool mSegmentsClickable = false;
};

#endif // NODESELECTORDESTINATIONBREADCRUMB_H
