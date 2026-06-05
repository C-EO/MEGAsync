#include "NodeSelectorSelectTypes.h"

#include "MegaApplication.h"
#include "NodeSelectorProxyModel.h"
#include "NodeSelectorTreeView.h"
#include "NodeSelectorTreeViewWidget.h"
#include "NodeSelectorTreeViewWidgetSpecializations.h"
#include "ui_NodeSelectorTreeViewWidget.h"

#include <QCoreApplication>

#include <algorithm>

///////////////////////SELECT TYPE//////////////////////////////
SelectType::SelectType()
{
    qRegisterMetaType<TabTypes>("TabTypes");
}

bool SelectType::isAllowedToNavigateInside(const QModelIndex& index)
{
    auto item = NodeSelectorModel::getItemByIndex(index);
    if (!item)
    {
        return false;
    }
    return !(item->getNode()->isFile() || item->isCloudDrive() || item->isRubbishBin() ||
             item->isTakenDown());
}

bool SelectType::isDownloadAllowed() const
{
    return false;
}

bool SelectType::okButtonEnabled(const QModelIndexList& selected)
{
    return !selected.isEmpty() &&
           std::none_of(
               selected.cbegin(),
               selected.cend(),
               [](const QModelIndex& index)
               {
                   return index.data(toInt(NodeSelectorModelRoles::IS_TAKEN_DOWN_ROLE)).toBool();
               });
}

QString SelectType::okButtonText() const
{
    return QCoreApplication::translate("NodeSelectorTreeViewWidget", "Ok");
}

void SelectType::initTreeViewWidget(NodeSelectorTreeViewWidget* wdg)
{
    wdg->ui->tMegaFolders->setAllowContextMenu(areActionsAllowed());
    wdg->ui->tMegaFolders->setAllowNewFolderContextMenuItem(hasNewFolderButton());

    wdg->mModel->showFiles(true);
    wdg->mModel->showReadOnlyFolders(true);

    if (isFilePicker())
    {
        wdg->ui->tMegaFolders->header()->setProperty("class", QLatin1String("new-design"));
    }
}

void SelectType::updateActionButtonsText(QMap<uint, QPushButton*> buttons)
{
    if (!buttons.isEmpty())
    {
        for (auto it = buttons.keyValueBegin(); it != buttons.keyValueEnd(); ++it)
        {
            it->second->setText(getCustomButtonText(it->first));
        }
    }
}

QString SelectType::getCustomButtonText(uint buttonId) const
{
    switch (buttonId)
    {
        case ButtonId::NEW_FOLDER:
        {
            return QCoreApplication::translate("NodeSelectorTreeViewWidget", "Create folder");
        }
        default:
        {
            return QString();
        }
    }
}

std::shared_ptr<NodeSelectorProxyModel> SelectType::createProxyModel()
{
    return std::make_shared<NodeSelectorProxyModel>();
}

QSet<int> SelectType::nonInteractiveColumns() const
{
    return {};
}

void SelectType::checkActionButtonsVisibility(NodeSelectorTreeViewWidget* wdg,
                                              QMap<uint, QPushButton*> buttons)
{
    // By default, all are hidden
    for (auto it = buttons.cbegin(); it != buttons.cend(); ++it)
    {
        checkActionButtonVisibility(wdg, it.key(), it.value());
    }
}

void SelectType::checkActionButtonVisibility(NodeSelectorTreeViewWidget* wdg,
                                             uint buttonId,
                                             QPushButton* button)
{
    if (button)
    {
        button->setVisible(checkActionButtonVisibility(wdg, buttonId));
    }
}

bool SelectType::checkActionButtonVisibility(NodeSelectorTreeViewWidget* wdg, uint buttonId)
{
    if (buttonId == SelectType::ButtonId::NEW_FOLDER)
    {
        return hasNewFolderButton() && !wdg->isCurrentRootIndexReadOnly() &&
               !wdg->isCurrentSelectionReadOnly();
    }
    else
    {
        return false;
    }
}

bool SelectType::cloudDriveIsCurrentRootIndex(NodeSelectorTreeViewWidget* wdg)
{
    auto rootItem = wdg->rootItem();
    return rootItem && rootItem->isCloudDrive();
}

bool SelectType::showEmptyStateUploadButton(NodeSelectorTreeViewWidget*) const
{
    return false;
}

bool SelectType::showEmptyStateNewFolderButton(NodeSelectorTreeViewWidget* wdg) const
{
    return wdg && hasNewFolderButton() && !wdg->isCurrentRootIndexReadOnly() &&
           !wdg->isCurrentSelectionReadOnly();
}

bool SelectType::showEmptyStateAddBackupButton(NodeSelectorTreeViewWidget*) const
{
    return false;
}

///////////////////////SELECT TYPE//////////////////////////////

///////////////////////CLOUD DRIVE TYPE//////////////////////////////
bool CloudDriveType::isDownloadAllowed() const
{
    return true;
}

void CloudDriveType::initTreeViewWidget(NodeSelectorTreeViewWidget* wdg)
{
    SelectType::initTreeViewWidget(wdg);

    wdg->ui->tMegaFolders->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

TabTypes CloudDriveType::allowedTabTypes()
{
    return TabType::CLOUD_DRIVE | TabType::INCOMING_SHARE | TabType::BACKUP | TabType::RUBBISH;
}

SelectType::EmptyPageInfo CloudDriveType::getEmptyFolderPageInfo() const
{
    EmptyPageInfo info;
    info.title = NodeSelectorTreeViewWidget::tr(
        "This folder is empty. Keep something safe with zero-knowledge encryption.");
    info.description = NodeSelectorTreeViewWidget::tr("Drag and drop your files here");
    info.icon.addFile(
        Utilities::getPixmapName(QLatin1String("empty_safe"), Utilities::AttributeType::NONE));
    info.buttons = ButtonId::UPLOAD;
    info.hasBorder = true;
    return info;
}

SelectType::EmptyPageInfo CloudDriveType::getEmptyCloudDrivePage() const
{
    SelectType::EmptyPageInfo info;
    info.title = NodeSelectorTreeViewWidget::tr("Nothing in your private vault yet");
    info.description = NodeSelectorTreeViewWidget::tr("Drag and drop your files here");
    info.icon.addFile(Utilities::getPixmapName(QLatin1String("empty_cloud_with_add_action"),
                                               Utilities::AttributeType::NONE));

    info.buttons = ButtonId::NEW_FOLDER | ButtonId::UPLOAD;

    return info;
}

bool CloudDriveType::okButtonEnabled(const QModelIndexList&)
{
    return false;
}

QString CloudDriveType::getCustomButtonText(uint buttonId) const
{
    switch (buttonId)
    {
        case ButtonId::UPLOAD:
        {
            return MegaApplication::tr("Upload");
        }
        case ButtonId::CLEAR_RUBBISH:
        {
            return NodeSelectorTreeViewWidget::tr("Empty Rubbish bin");
        }
        case ButtonId::ADD_BACKUP:
        {
            return NodeSelectorTreeViewWidget::tr("Add backup");
        }
        default:
        {
            return SelectType::getCustomButtonText(buttonId);
        }
    }
}

bool CloudDriveType::showEmptyStateUploadButton(NodeSelectorTreeViewWidget* wdg) const
{
    return wdg && wdg->getTabType() == NodeSelectorTreeViewWidget::TabItem::CLOUD_DRIVE &&
           !wdg->isCurrentRootIndexReadOnly();
}

bool CloudDriveType::showEmptyStateAddBackupButton(NodeSelectorTreeViewWidget* wdg) const
{
    return wdg && wdg->getTabType() == NodeSelectorTreeViewWidget::TabItem::BACKUPS;
}

bool CloudDriveType::checkActionButtonVisibility(NodeSelectorTreeViewWidget* wdg, uint buttonId)
{
    auto result(false);

    if (!wdg)
    {
        return result;
    }

    const bool isRubbish = wdg->getTabType() == NodeSelectorTreeViewWidget::TabItem::RUBBISH;

    switch (buttonId)
    {
        case SelectType::ButtonId::UPLOAD:
        {
            result = isRubbish ? false : !wdg->isCurrentRootIndexReadOnly();
            break;
        }
        case SelectType::ButtonId::CLEAR_RUBBISH:
        {
            result = isRubbish ? !wdg->isEmpty() : false;
            break;
        }
        case SelectType::ButtonId::ADD_BACKUP:
        {
            result = wdg->getTabType() == NodeSelectorTreeViewWidget::TabItem::BACKUPS &&
                     !wdg->isEmpty();
            break;
        }
        default:
        {
            result = SelectType::checkActionButtonVisibility(wdg, buttonId);
        }
    }

    return result;
}

bool CloudDriveType::acceptDrops(int tabItem)
{
    return tabItem == NodeSelectorTreeViewWidget::TabItem::CLOUD_DRIVE |
           tabItem == NodeSelectorTreeViewWidget::TabItem::SHARES |
           tabItem == NodeSelectorTreeViewWidget::TabItem::RUBBISH;
}

///////////////////////CLOUD DRIVE TYPE//////////////////////////////

///////////////////////FILE PICKER TYPE/////////////////////////
SelectType::EmptyPageInfo FilePickerType::getEmptyCloudDrivePage() const
{
    SelectType::EmptyPageInfo info;
    info.title = NodeSelectorTreeViewWidget::tr("No folders in Cloud drive");
    info.description = NodeSelectorTreeViewWidget::tr("Create a folder to move your content here");
    info.icon.addFile(Utilities::getPixmapName(QLatin1String("empty_cloud_drive"),
                                               Utilities::AttributeType::NONE));

    return info;
}

SelectType::EmptyPageInfo FilePickerType::getEmptyFolderPageInfo() const
{
    EmptyPageInfo info;
    info.title = NodeSelectorTreeViewWidget::tr(
        "This folder is empty. Keep something safe with zero-knowledge encryption.");
    info.icon.addFile(
        Utilities::getPixmapName(QLatin1String("empty_safe"), Utilities::AttributeType::NONE));
    return info;
}

QSet<int> FilePickerType::nonInteractiveColumns() const
{
    // Only NODE carries a header label and is sortable; mark every other column non-interactive
    // (no name, no sort).
    QSet<int> columns;
    for (int column = NodeSelectorModel::Column::NODE + 1; column < NodeSelectorModel::Column::last;
         ++column)
    {
        columns.insert(column);
    }
    return columns;
}

///////////////////////FILE PICKER TYPE/////////////////////////

///////////////////////SYNC TYPE//////////////////////////////
void SyncType::initTreeViewWidget(NodeSelectorTreeViewWidget* wdg)
{
    SelectType::initTreeViewWidget(wdg);

    wdg->mModel->setSyncSetupMode(true);
    wdg->mModel->showFiles(false);
    wdg->mModel->showReadOnlyFolders(false);
}

bool SyncType::okButtonEnabled(const QModelIndexList& selected)
{
    if (selected.size() != 1)
    {
        return false;
    }

    bool isSyncable =
        selected.first().data(toInt(NodeSelectorModelRoles::IS_SYNCABLE_FOLDER_ROLE)).toBool();
    bool isFile = selected.first().data(toInt(NodeSelectorModelRoles::IS_FILE_ROLE)).toBool();
    return (isSyncable && !isFile) && SelectType::okButtonEnabled(selected);
}

TabTypes SyncType::allowedTabTypes()
{
    return TabType::CLOUD_DRIVE | TabType::INCOMING_SHARE;
}

QString SyncType::okButtonText() const
{
    return QCoreApplication::translate("NodeSelectorTreeViewWidget", "Sync");
}

SelectType::EmptyPageInfo SyncType::getEmptyFolderPageInfo() const
{
    EmptyPageInfo info;
    info.title = NodeSelectorTreeViewWidget::tr("No folders to select");
    info.description = NodeSelectorTreeViewWidget::tr("Only folders can be synced");
    info.icon = Utilities::getIcon(QLatin1String("synced-folder"), Utilities::AttributeType::NONE);
    return info;
}

std::shared_ptr<NodeSelectorProxyModel> SyncType::createProxyModel()
{
    return std::make_shared<NodeSelectorProxyModelSync>();
}

///////////////////////SYNC TYPE//////////////////////////////

///////////////////////STREAM TYPE//////////////////////////////
bool StreamType::okButtonEnabled(const QModelIndexList& selected)
{
    if (selected.size() != 1)
    {
        return false;
    }

    return selected.first().data(toInt(NodeSelectorModelRoles::IS_FILE_ROLE)).toBool() ?
               SelectType::okButtonEnabled(selected) :
               false;
}

TabTypes StreamType::allowedTabTypes()
{
    return TabType::CLOUD_DRIVE | TabType::INCOMING_SHARE | TabType::BACKUP;
}

///////////////////////STREAM TYPE//////////////////////////////

///////////////////////UPLOAD TYPE//////////////////////////////
void UploadType::initTreeViewWidget(NodeSelectorTreeViewWidget* wdg)
{
    SelectType::initTreeViewWidget(wdg);

    wdg->mModel->showFiles(false);
    wdg->mModel->showReadOnlyFolders(false);
}

bool UploadType::okButtonEnabled(const QModelIndexList& selected)
{
    if (selected.size() != 1)
    {
        return false;
    }

    return selected.first().data(toInt(NodeSelectorModelRoles::IS_FILE_ROLE)).toBool() ?
               false :
               SelectType::okButtonEnabled(selected);
}

TabTypes UploadType::allowedTabTypes()
{
    return TabType::CLOUD_DRIVE | TabType::INCOMING_SHARE;
}

QString UploadType::okButtonText() const
{
    return QCoreApplication::translate("NodeSelectorTreeViewWidget", "Upload");
}

///////////////////////UPLOAD TYPE//////////////////////////////

///////////////////////DOWNLOAD TYPE//////////////////////////////
void DownloadType::initTreeViewWidget(NodeSelectorTreeViewWidget* wdg)
{
    SelectType::initTreeViewWidget(wdg);

    wdg->ui->tMegaFolders->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

TabTypes DownloadType::allowedTabTypes()
{
    return TabType::CLOUD_DRIVE | TabType::INCOMING_SHARE | TabType::BACKUP;
}

bool DownloadType::isDownloadAllowed() const
{
    return true;
}

QString DownloadType::okButtonText() const
{
    return QCoreApplication::translate("NodeSelectorTreeViewWidget", "Download");
}

///////////////////////DOWNLOAD TYPE//////////////////////////////

///////////////////////MOVE BACKUP TYPE//////////////////////////////
TabTypes MoveBackupType::allowedTabTypes()
{
    return TabType::CLOUD_DRIVE;
}

QString MoveBackupType::okButtonText() const
{
    return QCoreApplication::translate("NodeSelectorTreeViewWidget", "Move");
}
///////////////////////MOVE BACKUP TYPE//////////////////////////////
