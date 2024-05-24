#include "LocalAndRemoteNameConflicts.h"
#include "ui_LocalAndRemoteNameConflicts.h"

#include <StalledIssueHeader.h>
#include <NameConflictStalledIssue.h>

const QString FILES_DESCRIPTION = QString::fromLatin1(QT_TRANSLATE_NOOP("LocalAndRemoteNameConflicts", "Renaming or removing files can resolve this issue,"
                                                                                              "\nor click the Folders below to make adjustments in the local filesystem or in MEGA"));
const QString FOLDERS_DESCRIPTION = QString::fromLatin1(QT_TRANSLATE_NOOP("LocalAndRemoteNameConflicts", "Renaming or removing folders can resolve this issue,"
                                                                                                "\nor click the Folders below to make adjustments in the local filesystem or in MEGA"));
const QString FILES_AND_FOLDERS_DESCRIPTION = QString::fromLatin1(QT_TRANSLATE_NOOP("LocalAndRemoteNameConflicts", "Renaming or removing files or folders can resolve this issue,"
                                                                                                          "\nor click the Folders below to make adjustments in the local filesystem or in MEGA"));

LocalAndRemoteNameConflicts::LocalAndRemoteNameConflicts(QWidget *parent) :
    StalledIssueBaseDelegateWidget(parent),
    ui(new Ui::LocalAndRemoteNameConflicts)
{
    ui->setupUi(this);

    auto margins = ui->verticalLayout->contentsMargins();
    margins.setLeft(StalledIssueHeader::GROUPBOX_INDENT);
    ui->verticalLayout->setContentsMargins(margins);

    connect(ui->cloudConflictNames, &NameConflict::refreshUi, this, &LocalAndRemoteNameConflicts::refreshUi);
    connect(ui->localConflictNames, &NameConflict::refreshUi, this, &LocalAndRemoteNameConflicts::refreshUi);

    connect(ui->cloudConflictNames, &NameConflict::allSolved, this, &LocalAndRemoteNameConflicts::onNameConflictSolved);
    connect(ui->localConflictNames, &NameConflict::allSolved, this, &LocalAndRemoteNameConflicts::onNameConflictSolved);

    ui->cloudConflictNames->setDelegate(this);
    ui->localConflictNames->setDelegate(this);
}

LocalAndRemoteNameConflicts::~LocalAndRemoteNameConflicts()
{
    delete ui;
}

void LocalAndRemoteNameConflicts::refreshUi()
{
    if(auto nameConflict =  getData().convert<NameConflictedStalledIssue>())
    {
        auto cloudData = nameConflict->getNameConflictCloudData();
        if(cloudData.isEmpty())
        {
            ui->cloudConflictNames->hide();
        }
        else
        {
            ui->cloudConflictNames->updateUi(nameConflict);
            ui->cloudConflictNames->show();
        }

        auto localData = nameConflict->getNameConflictLocalData();
        if(localData.isEmpty())
        {
            ui->localConflictNames->hide();
        }
        else
        {
            ui->localConflictNames->updateUi(nameConflict);
            ui->localConflictNames->show();
        }

        if(getData().consultData()->filesCount() > 0 && getData().consultData()->foldersCount() > 0)
        {
            ui->selectLabel->setText(FILES_AND_FOLDERS_DESCRIPTION);
        }
        else if(getData().consultData()->filesCount() > 0)
        {
            ui->selectLabel->setText(FILES_DESCRIPTION);
        }
        else if(getData().consultData()->foldersCount() > 0)
        {
            ui->selectLabel->setText(FOLDERS_DESCRIPTION);
        }
    }
}

void LocalAndRemoteNameConflicts::onNameConflictSolved()
{
    ui->cloudConflictNames->setDisabled();
    ui->localConflictNames->setDisabled();
}
