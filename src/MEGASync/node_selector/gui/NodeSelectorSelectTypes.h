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

    virtual bool isContextMenuAllowed()
    {
        return false;
    }

    virtual bool isAllowedToNavigateInside(const QModelIndex& index);
    virtual void init(NodeSelectorTreeViewWidget* wdg) = 0;
    virtual bool okButtonEnabled(NodeSelectorTreeViewWidget* wdg, const QModelIndexList& selected);

    virtual bool hasNewFolderButton() const
    {
        return false;
    }

    enum ButtonId : uint
    {
        Upload,
        NewFolder,
        ClearRubbish
    };

    virtual QMap<uint, QPushButton*> addActionButtons();
    virtual void updateCustomButtonsText(NodeSelectorTreeViewWidget*);
    virtual QString getCustomButtonText(uint buttonId) const;
    void checkActionButtonsVisibility(NodeSelectorTreeViewWidget* wdg);
    virtual void checkActionButtonVisibility(NodeSelectorTreeViewWidget* wdg,
                                             uint buttonId,
                                             QPushButton* button);

    virtual TabTypes allowedTabTypes() = 0;

    virtual bool footerVisible() const
    {
        return true;
    }

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

    QPushButton* createButton(const QString& type, const QString& text, const QString& iconFile);

    QMap<uint, QPushButton*> mActionButtons;
};

class DownloadType: public SelectType
{
public:
    explicit DownloadType() = default;
    void init(NodeSelectorTreeViewWidget* wdg) override;
    bool okButtonEnabled(NodeSelectorTreeViewWidget*, const QModelIndexList& selected) override;
    TabTypes allowedTabTypes() override;
};

class SyncType: public SelectType
{
public:
    explicit SyncType() = default;
    bool isAllowedToNavigateInside(const QModelIndex& index) override;
    void init(NodeSelectorTreeViewWidget* wdg) override;
    bool okButtonEnabled(NodeSelectorTreeViewWidget* wdg, const QModelIndexList& selected) override;
    TabTypes allowedTabTypes() override;
    EmptyFolderPageInfo getEmptyFolderPageInfo() override;
    std::shared_ptr<NodeSelectorProxyModel> createProxyModel() override;
};

class StreamType: public SelectType
{
public:
    explicit StreamType() = default;
    void init(NodeSelectorTreeViewWidget* wdg) override;
    bool okButtonEnabled(NodeSelectorTreeViewWidget*, const QModelIndexList& selected) override;
    TabTypes allowedTabTypes() override;
    std::shared_ptr<NodeSelectorProxyModel> createProxyModel() override;
};

class UploadType: public SelectType
{
public:
    explicit UploadType() = default;
    void init(NodeSelectorTreeViewWidget* wdg) override;
    bool okButtonEnabled(NodeSelectorTreeViewWidget* wdg, const QModelIndexList& selected) override;
    TabTypes allowedTabTypes() override;

    bool hasNewFolderButton() const override
    {
        return true;
    }
};

class CloudDriveType: public SelectType
{
public:
    explicit CloudDriveType() = default;

    bool isContextMenuAllowed() override
    {
        return true;
    }

    void init(NodeSelectorTreeViewWidget* wdg) override;
    QMap<uint, QPushButton*> addActionButtons() override;
    void checkActionButtonVisibility(NodeSelectorTreeViewWidget* wdg,
                                     uint buttonId,
                                     QPushButton* button) override;

    bool hasNewFolderButton() const override
    {
        return true;
    }

    bool okButtonEnabled(NodeSelectorTreeViewWidget*, const QModelIndexList& selected) override;
    TabTypes allowedTabTypes() override;
    bool footerVisible() const override;

private:
    QString getCustomButtonText(uint buttonId) const override;
};

class MoveBackupType: public UploadType
{
public:
    explicit MoveBackupType() = default;
    bool okButtonEnabled(NodeSelectorTreeViewWidget* wdg, const QModelIndexList& selected) override;
    TabTypes allowedTabTypes() override;
};

#endif // NODESELECTORSELECTTYPES_H
