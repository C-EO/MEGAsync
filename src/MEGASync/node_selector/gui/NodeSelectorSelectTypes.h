#ifndef NODESELECTORSELECTTYPES_H
#define NODESELECTORSELECTTYPES_H

#include "NodeSelectorTabTypes.h"

#include <QIcon>
#include <QMap>
#include <QModelIndex>
#include <QPushButton>
#include <QString>

#include <memory>

class NodeSelectorProxyModel;
class NodeSelectorTreeViewWidget;
class NodeSelectorTreeView;
class SelectType;
typedef std::shared_ptr<SelectType> SelectTypeSPtr;

class SelectType
{
public:
    explicit SelectType();
    virtual ~SelectType() = default;

    virtual bool areActionsAllowed()
    {
        return false;
    }

    virtual bool isCurrentFolderValidForSelection() const
    {
        return true;
    }
    virtual bool isAllowedToNavigateInside(const QModelIndex& index);
    virtual bool isDownloadAllowed() const;
    virtual void initTreeViewWidget(NodeSelectorTreeViewWidget* wdg);
    virtual bool okButtonEnabled(const QModelIndexList& selected);

    virtual bool hasNewFolderButton() const
    {
        return false;
    }

    virtual bool flattenSearchResults() const
    {
        return false;
    }

    virtual bool isFilePicker() const
    {
        return true;
    }

    virtual bool showsDestinationBreadcrumb() const
    {
        return false;
    }

    enum ButtonId : uint
    {
        Upload,
        NewFolder,
        ClearRubbish
    };

    virtual void updateActionButtonsText(QMap<uint, QPushButton*> buttons);
    virtual QString getCustomButtonText(uint buttonId) const;
    void checkActionButtonsVisibility(NodeSelectorTreeViewWidget* wdg,
                                      QMap<uint, QPushButton*> buttons);
    virtual void checkActionButtonVisibility(NodeSelectorTreeViewWidget* wdg,
                                             uint buttonId,
                                             QPushButton* button);

    virtual TabTypes allowedTabTypes() = 0;

    struct EmptyFolderPageInfo
    {
        QString title;
        QString description;
        QIcon icon;
        bool iconTokenized = true;
        QString descriptionLabelFontSize;

        bool isValid()
        {
            return !title.isEmpty() && !description.isEmpty() && !icon.isNull() &&
                   !descriptionLabelFontSize.isEmpty();
        }
    };

    virtual EmptyFolderPageInfo getEmptyFolderPageInfo()
    {
        return EmptyFolderPageInfo();
    }

    virtual std::shared_ptr<NodeSelectorProxyModel> createProxyModel();

protected:
    bool cloudDriveIsCurrentRootIndex(NodeSelectorTreeViewWidget* wdg);
};

class DownloadType: public SelectType
{
public:
    explicit DownloadType() = default;
    void initTreeViewWidget(NodeSelectorTreeViewWidget* wdg) override;
    TabTypes allowedTabTypes() override;
    bool isDownloadAllowed() const override;
};

class SyncType: public SelectType
{
public:
    explicit SyncType() = default;
    bool isAllowedToNavigateInside(const QModelIndex& index) override;
    void initTreeViewWidget(NodeSelectorTreeViewWidget* wdg) override;
    bool okButtonEnabled(const QModelIndexList& selected) override;
    TabTypes allowedTabTypes() override;
    EmptyFolderPageInfo getEmptyFolderPageInfo() override;
    std::shared_ptr<NodeSelectorProxyModel> createProxyModel() override;

    bool hasNewFolderButton() const override
    {
        return true;
    }

    bool showsDestinationBreadcrumb() const override
    {
        return true;
    }
};

class StreamType: public SelectType
{
public:
    explicit StreamType() = default;
    bool okButtonEnabled(const QModelIndexList& selected) override;
    TabTypes allowedTabTypes() override;

    bool showsDestinationBreadcrumb() const override
    {
        return false;
    }

    bool isCurrentFolderValidForSelection() const override
    {
        return false;
    }
};

class UploadType: public SelectType
{
public:
    explicit UploadType() = default;
    void initTreeViewWidget(NodeSelectorTreeViewWidget* wdg) override;
    bool okButtonEnabled(const QModelIndexList& selected) override;
    TabTypes allowedTabTypes() override;

    bool hasNewFolderButton() const override
    {
        return true;
    }

    bool showsDestinationBreadcrumb() const override
    {
        return true;
    }
};

class CloudDriveType: public SelectType
{
public:
    explicit CloudDriveType() = default;

    bool areActionsAllowed() override
    {
        return true;
    }

    bool isDownloadAllowed() const override;

    void initTreeViewWidget(NodeSelectorTreeViewWidget* wdg) override;
    void checkActionButtonVisibility(NodeSelectorTreeViewWidget* wdg,
                                     uint buttonId,
                                     QPushButton* button) override;

    bool hasNewFolderButton() const override
    {
        return true;
    }

    bool okButtonEnabled(const QModelIndexList& selected) override;
    TabTypes allowedTabTypes() override;
    bool flattenSearchResults() const override
    {
        return false;
    }

    bool isFilePicker() const override
    {
        return false;
    }

private:
    QString getCustomButtonText(uint buttonId) const override;
};

class MoveBackupType: public UploadType
{
public:
    explicit MoveBackupType() = default;
    TabTypes allowedTabTypes() override;
};

#endif // NODESELECTORSELECTTYPES_H
