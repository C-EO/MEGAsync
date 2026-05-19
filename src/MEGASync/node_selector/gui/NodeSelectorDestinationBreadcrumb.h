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
    explicit NodeSelectorDestinationBreadcrumb(QWidget* parent = nullptr);
    ~NodeSelectorDestinationBreadcrumb() override;

    void setPathSegments(const QStringList& segments);
    void setTitleText(const QString& text);
    void showDefaultUploadOption(bool show = true);
    void setDefaultUploadOption(bool value);
    bool getDefaultUploadOption() const;

signals:
    void clearRequested();

private:
    void closeOverflowPopup();
    void showOverflowPopup();
    void rebuildSegments();
    void clearSegmentWidgets();
    QLabel* makeSegmentLabel(const QString& text, bool isFirst, bool isLast);
    QLabel* makeSeparatorLabel();
    void updateContentVisibility();

    Ui::NodeSelectorDestinationBreadcrumb* ui;
    QPointer<NodeSelectorDestinationOverflowPopup> mOverflowPopup;
    QStringList mPathSegments;
    bool mShouldShowDefaultUploadOption = false;
};

#endif // NODESELECTORDESTINATIONBREADCRUMB_H
