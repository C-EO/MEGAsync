#include "LocalAndRemoteDifferentWidget.h"
#include "ui_LocalAndRemoteDifferentWidget.h"

#include "MegaApplication.h"
#include "StalledIssuesModel.h"
#include "StalledIssueHeader.h"

#include "mega/types.h"

#include <QFile>

LocalAndRemoteDifferentWidget::LocalAndRemoteDifferentWidget(QWidget *parent) :
    StalledIssueBaseDelegateWidget(parent), mega::MegaRequestListener(),
    ui(new Ui::LocalAndRemoteDifferentWidget),
    mListener(mega::make_unique<mega::QTMegaRequestListener>(MegaSyncApp->getMegaApi(), this)),
    mRemovedRemoteHandle(0)
{
    ui->setupUi(this);

    connect(ui->chooseLocalCopy, &StalledIssueChooseWidget::chooseButtonClicked, this, &LocalAndRemoteDifferentWidget::onLocalButtonClicked);
    connect(ui->chooseRemoteCopy, &StalledIssueChooseWidget::chooseButtonClicked, this, &LocalAndRemoteDifferentWidget::onRemoteButtonClicked);

    auto margins = ui->verticalLayout->contentsMargins();
    margins.setLeft(StalledIssueHeader::ARROW_INDENT);
    ui->verticalLayout->setContentsMargins(margins);

    ui->selectLabel->setIndent(StalledIssueHeader::ICON_INDENT);
}

LocalAndRemoteDifferentWidget::~LocalAndRemoteDifferentWidget()
{
    delete ui;
}

void LocalAndRemoteDifferentWidget::refreshUi()
{
    auto issue = getData();
    for(int index = 0; index < issue.stalledIssuesCount(); ++index)
    {
        auto data = issue.getStalledIssueData(index);
        data->mIsCloud ? ui->chooseRemoteCopy->setData(data, issue.getFileName()) : ui->chooseLocalCopy->setData(data, issue.getFileName());
    }
}

void LocalAndRemoteDifferentWidget::onRequestFinish(mega::MegaApi *, mega::MegaRequest *request, mega::MegaError *e)
{
    if (request->getType() == mega::MegaRequest::TYPE_REMOVE)
    {
        if (e->getErrorCode() == mega::MegaError::API_OK)
        {
            auto handle = request->getNodeHandle();
            if(handle && handle == mRemovedRemoteHandle)
            {
                emit issueFixed();
                mRemovedRemoteHandle = 0;
            }
        }
    }
}

void LocalAndRemoteDifferentWidget::onLocalButtonClicked()
{ 
    auto fileNode(MegaSyncApp->getMegaApi()->getNodeByPath(ui->chooseRemoteCopy->data()->mIndexPath.path.toStdString().c_str()));
    if(fileNode)
    {
        mRemovedRemoteHandle = fileNode->getHandle();
        MegaSyncApp->getMegaApi()->remove(fileNode, mListener.get());
    }
}

void LocalAndRemoteDifferentWidget::onRemoteButtonClicked()
{
    QFile file(ui->chooseLocalCopy->data()->mIndexPath.path);
    if(file.exists())
    {
        if(file.remove())
        {
            emit issueFixed();
        }
    }
}
