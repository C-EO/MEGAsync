#ifndef FILEPICKERNODESELECTOR_H
#define FILEPICKERNODESELECTOR_H

#include "NodeSelector.h"

class FilePickerNodeSelector: public NodeSelector
{
    Q_OBJECT

public:
    explicit FilePickerNodeSelector(SelectTypeSPtr selectType, QWidget* parent = nullptr);

protected:
    ClearTypes searchClearType() const override;
    void handleSearch(const QString& text) override;
    void handleSearchHidden() override;

    bool searchHasOwnTab() const override
    {
        return false;
    }

    void configureCloudDriveWidget() override;
    void configureIncomingSharesWidget() override;
    void configureBackupsWidget() override;
    void configureRubbishWidget() override;
    void configureSearchWidget(TabType type) override;

protected slots:
    void onItemsAboutToBeMoved(const QList<mega::MegaHandle>& handles, int actionType) override;
    void onItemsAboutToBeMovedFailed(const QList<mega::MegaHandle>& handles, int type) override;

private:
    void hideAllButNode(NodeSelectorTreeViewWidget* widget);
};

#endif // FILEPICKERNODESELECTOR_H
