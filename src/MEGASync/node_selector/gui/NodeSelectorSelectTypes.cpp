#include "NodeSelectorSelectTypes.h"

#include "MegaApplication.h"
#include "NodeSelectorProxyModel.h"
#include "NodeSelectorTreeView.h"
#include "NodeSelectorTreeViewWidget.h"
#include "NodeSelectorTreeViewWidgetSpecializations.h"
#include "TokenizableItems/TokenizableButtons.h"
#include "ui_NodeSelectorTreeViewWidget.h"

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

bool SelectType::okButtonEnabled(NodeSelectorTreeViewWidget*, const QModelIndexList& selected)
{
    return std::none_of(
        selected.cbegin(),
        selected.cend(),
        [](const QModelIndex& index)
        {
            return index.data(toInt(NodeSelectorModelRoles::IS_TAKEN_DOWN_ROLE)).toBool();
        });
}

QMap<uint, QPushButton*> SelectType::addActionButtons()
{
    if (hasNewFolderButton())
    {
        auto newFolder =
            createButton(QLatin1String("ghost"),
                         getCustomButtonText(ButtonId::NewFolder),
                         Utilities::getPixmapName(QLatin1String("folder-plus-01"),
                                                  Utilities::AttributeType::SMALL |
                                                      Utilities::AttributeType::THIN |
                                                      Utilities::AttributeType::OUTLINE));

        mActionButtons.insert(ButtonId::NewFolder, newFolder);
    }

    return mActionButtons;
}

void SelectType::updateCustomButtonsText(NodeSelectorTreeViewWidget*)
{
    if (!mActionButtons.isEmpty())
    {
        for (auto it = mActionButtons.keyValueBegin(); it != mActionButtons.keyValueEnd(); ++it)
        {
            it->second->setText(getCustomButtonText(it->first));
        }
    }
}

QString SelectType::getCustomButtonText(uint buttonId) const
{
    switch (buttonId)
    {
        case ButtonId::NewFolder:
        {
            return MegaApplication::tr("New Folder");
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

void SelectType::checkActionButtonsVisibility(NodeSelectorTreeViewWidget* wdg)
{
    // By default, all are hidden
    for (auto it = mActionButtons.cbegin(); it != mActionButtons.cend(); ++it)
    {
        checkActionButtonVisibility(wdg, it.key(), it.value());
    }
}

void SelectType::checkActionButtonVisibility(NodeSelectorTreeViewWidget* wdg,
                                             uint buttonId,
                                             QPushButton* button)
{
    if (buttonId == SelectType::ButtonId::NewFolder)
    {
        button->setVisible(!wdg->isEmpty() && !wdg->isCurrentRootIndexReadOnly() &&
                           !wdg->isCurrentSelectionReadOnly());
    }
    else
    {
        button->hide();
    }
}

bool SelectType::cloudDriveIsCurrentRootIndex(NodeSelectorTreeViewWidget* wdg)
{
    auto rootItem = wdg->rootItem();
    return rootItem && rootItem->isCloudDrive();
}

QPushButton* SelectType::createButton(const QString& type,
                                      const QString& text,
                                      const QString& iconFile)
{
    auto button(new TokenizableButton());
    button->setText(text);
    button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QIcon icon;
    icon.addFile(iconFile, QSize(16, 16), QIcon::Mode::Normal, QIcon::State::Off);
    button->setIcon(icon);
    button->setProperty("type", type);
    button->setProperty("dimension", QLatin1String("small"));
    return button;
}

///////////////////////SELECT TYPE//////////////////////////////

///////////////////////SYNC TYPE//////////////////////////////
void SyncType::init(NodeSelectorTreeViewWidget* wdg)
{
    wdg->mModel->setSyncSetupMode(true);
    wdg->mModel->showFiles(false);
    wdg->mModel->showReadOnlyFolders(false);
}

bool SyncType::okButtonEnabled(NodeSelectorTreeViewWidget* wdg, const QModelIndexList& selected)
{
    if (selected.size() == 1 && SelectType::okButtonEnabled(wdg, selected))
    {
        bool isSyncable =
            selected.first().data(toInt(NodeSelectorModelRoles::IS_SYNCABLE_FOLDER_ROLE)).toBool();
        bool isFile = selected.first().data(toInt(NodeSelectorModelRoles::IS_FILE_ROLE)).toBool();
        return isSyncable && !isFile;
    }

    return false;
}

TabTypes SyncType::allowedTabTypes()
{
    return TabType::CLOUD_DRIVE | TabType::INCOMING_SHARE;
}

SelectType::EmptyFolderPageInfo SyncType::getEmptyFolderPageInfo()
{
    EmptyFolderPageInfo info;
    info.title = NodeSelectorTreeViewWidget::tr("No folders to select");
    info.description = NodeSelectorTreeViewWidget::tr("Only folders can be synced");
    info.icon = Utilities::getIcon(QLatin1String("synced-folder"), Utilities::AttributeType::NONE);
    info.descriptionLabelFontSize = QLatin1String("body-1");
    info.iconTokenized = false;
    return info;
}

std::shared_ptr<NodeSelectorProxyModel> SyncType::createProxyModel()
{
    return std::make_shared<NodeSelectorProxyModelSync>();
}

bool SyncType::isAllowedToNavigateInside(const QModelIndex& index)
{
    if (!SelectType::isAllowedToNavigateInside(index))
    {
        return false;
    }
    auto item = NodeSelectorModel::getItemByIndex(index);
    return !(item->getStatus() == NodeSelectorModelItem::Status::SYNC ||
             item->getStatus() == NodeSelectorModelItem::Status::SYNC_CHILD);
}

///////////////////////SYNC TYPE//////////////////////////////

///////////////////////STREAM TYPE//////////////////////////////
void StreamType::init(NodeSelectorTreeViewWidget* wdg)
{
    wdg->mModel->showFiles(true);
    wdg->mModel->showReadOnlyFolders(true);
}

bool StreamType::okButtonEnabled(NodeSelectorTreeViewWidget* wdg, const QModelIndexList& selected)
{
    if (selected.size() == 1 && SelectType::okButtonEnabled(wdg, selected))
    {
        return selected.first().data(toInt(NodeSelectorModelRoles::IS_FILE_ROLE)).toBool();
    }

    return false;
}

TabTypes StreamType::allowedTabTypes()
{
    return TabType::CLOUD_DRIVE | TabType::INCOMING_SHARE | TabType::BACKUP;
}

std::shared_ptr<NodeSelectorProxyModel> StreamType::createProxyModel()
{
    return std::make_shared<NodeSelectorProxyModelStream>();
}

///////////////////////STREAM TYPE//////////////////////////////

///////////////////////UPLOAD TYPE//////////////////////////////
void UploadType::init(NodeSelectorTreeViewWidget* wdg)
{
    wdg->mModel->showFiles(false);
    wdg->mModel->showReadOnlyFolders(false);
}

bool UploadType::okButtonEnabled(NodeSelectorTreeViewWidget* wdg, const QModelIndexList& selected)
{
    auto itemsSelected(selected.size());
    // If we have one or less items selected
    if (itemsSelected < 2)
    {
        return itemsSelected == 1 ? SelectType::okButtonEnabled(wdg, selected) &&
                                        !selected.first()
                                             .data(toInt(NodeSelectorModelRoles::IS_FILE_ROLE))
                                             .toBool() :
                                    !wdg->isCurrentRootIndexReadOnly();
    }

    // If we have more than one item selected, we cannot upload anything
    return false;
}

TabTypes UploadType::allowedTabTypes()
{
    return TabType::CLOUD_DRIVE | TabType::INCOMING_SHARE;
}

///////////////////////UPLOAD TYPE//////////////////////////////

///////////////////////DOWNLOAD TYPE//////////////////////////////
void DownloadType::init(NodeSelectorTreeViewWidget* wdg)
{
    wdg->ui->tMegaFolders->setSelectionMode(QAbstractItemView::ExtendedSelection);
    wdg->mModel->showFiles(true);
    wdg->mModel->showReadOnlyFolders(true);
}

bool DownloadType::okButtonEnabled(NodeSelectorTreeViewWidget* wdg, const QModelIndexList& selected)
{
    return SelectType::okButtonEnabled(wdg, selected) &&
           (!selected.isEmpty() || cloudDriveIsCurrentRootIndex(wdg));
}

TabTypes DownloadType::allowedTabTypes()
{
    return TabType::CLOUD_DRIVE | TabType::INCOMING_SHARE | TabType::BACKUP;
}

///////////////////////DOWNLOAD TYPE//////////////////////////////

///////////////////////CLOUD DRIVE TYPE//////////////////////////////
void CloudDriveType::init(NodeSelectorTreeViewWidget* wdg)
{
    wdg->ui->tMegaFolders->setSelectionMode(QAbstractItemView::ExtendedSelection);
    wdg->mModel->showFiles(true);
    wdg->mModel->showReadOnlyFolders(true);
}

TabTypes CloudDriveType::allowedTabTypes()
{
    return TabType::CLOUD_DRIVE | TabType::INCOMING_SHARE | TabType::BACKUP | TabType::RUBBISH;
}

bool CloudDriveType::footerVisible() const
{
    return false;
}

bool CloudDriveType::okButtonEnabled(NodeSelectorTreeViewWidget*, const QModelIndexList&)
{
    return false;
}

QMap<uint, QPushButton*> CloudDriveType::addActionButtons()
{
    auto uploadButton = createButton(
        QLatin1String("ghost"),
        getCustomButtonText(ButtonId::Upload),
        Utilities::getPixmapName(QLatin1String("arrow-up"),
                                 Utilities::AttributeType::SMALL | Utilities::AttributeType::THIN |
                                     Utilities::AttributeType::OUTLINE));

    mActionButtons.insert(ButtonId::Upload, uploadButton);

    auto clearRubbishButton = createButton(
        QLatin1String("ghost"),
        getCustomButtonText(ButtonId::ClearRubbish),
        Utilities::getPixmapName(QLatin1String("x"),
                                 Utilities::AttributeType::SMALL | Utilities::AttributeType::THIN |
                                     Utilities::AttributeType::OUTLINE));
    mActionButtons.insert(ButtonId::ClearRubbish, clearRubbishButton);

    SelectType::addActionButtons();

    return mActionButtons;
}

QString CloudDriveType::getCustomButtonText(uint buttonId) const
{
    switch (buttonId)
    {
        case ButtonId::Upload:
        {
            return MegaApplication::tr("Upload");
        }
        case ButtonId::ClearRubbish:
        {
            return NodeSelectorTreeViewWidget::tr("Empty Rubbish bin");
        }
        default:
        {
            return SelectType::getCustomButtonText(buttonId);
        }
    }
}

void CloudDriveType::checkActionButtonVisibility(NodeSelectorTreeViewWidget* wdg,
                                                 uint buttonId,
                                                 QPushButton* button)
{
    auto rubbishWidget = dynamic_cast<NodeSelectorTreeViewWidgetRubbish*>(wdg);

    switch (buttonId)
    {
        case SelectType::ButtonId::Upload:
        {
            rubbishWidget ? button->setVisible(false) :
                            button->setVisible(!wdg->isCurrentRootIndexReadOnly());
            break;
        }
        case SelectType::ButtonId::ClearRubbish:
        {
            rubbishWidget ? button->setVisible(!rubbishWidget->isEmpty()) :
                            button->setVisible(false);
            break;
        }
        default:
        {
            SelectType::checkActionButtonVisibility(wdg, buttonId, button);
        }
    }
}

///////////////////////CLOUD DRIVE TYPE//////////////////////////////

///////////////////////MOVE BACKUP TYPE//////////////////////////////
TabTypes MoveBackupType::allowedTabTypes()
{
    return TabType::CLOUD_DRIVE;
}

bool MoveBackupType::okButtonEnabled(NodeSelectorTreeViewWidget* wdg,
                                     const QModelIndexList& selected)
{
    return selected.size() == 1 && SelectType::okButtonEnabled(wdg, selected);
}

///////////////////////MOVE BACKUP TYPE//////////////////////////////
