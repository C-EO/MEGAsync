#include "StalledIssueTab.h"
#include "ui_StalledIssueTab.h"

#include "Utilities.h"
#include "StalledIssuesModel.h"
#include "MegaApplication.h"

const char* StalledIssueTab::HOVER_PROPERTY = "itsHover";

StalledIssueTab::StalledIssueTab(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::StalledIssueTab),
    mItsOn(false),
    mFilterCriterion(-1),
    mShadowTab (new QGraphicsDropShadowEffect(nullptr))
{
    ui->setupUi(this);

    connect(MegaSyncApp->getStalledIssuesModel(),
            &StalledIssuesModel::stalledIssuesCountChanged, this, &StalledIssueTab::onUpdateCounter);
}

StalledIssueTab::~StalledIssueTab()
{
    delete ui;
}

void StalledIssueTab::setTitle(const QString &title)
{
    mTitle = title;
    ui->title->setText(title);
}

void StalledIssueTab::setIconPrefix(const QString &iconPrefix)
{
    mIconPrefix = iconPrefix;
    updateIcon();
}

void StalledIssueTab::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        //If turned off, turn on and turn off siblings
        if(!itsOn())
        {
            toggleOffSiblings();

            setItsOn(true);

            updateIcon();

            //We don´t want to propagate it to the stalled issue dialog
            return;
        }
    }

    QWidget::mouseReleaseEvent(event);
}

void StalledIssueTab::enterEvent(QEvent*)
{
   if(isEnabled())
   {
       setProperty(HOVER_PROPERTY,true);
       setStyleSheet(styleSheet());

       if(!mIconPrefix.isEmpty())
       {
           QString iconName =  QString::fromUtf8(":images/StalledIssues/") + mIconPrefix
                   + QString::fromUtf8("-solid")
                   + QString::fromUtf8(".png");
           QIcon icon = Utilities::getCachedPixmap(iconName);

           ui->icon->setPixmap(icon.pixmap(ui->icon->size()));
       }
   }
}

void StalledIssueTab::leaveEvent(QEvent*)
{
    setProperty(HOVER_PROPERTY,false);
    setStyleSheet(styleSheet());
    updateIcon();
}

void StalledIssueTab::onUpdateCounter()
{
    ui->title->setText(createTitle());
}

bool StalledIssueTab::itsOn() const
{
    return mItsOn;
}

void StalledIssueTab::setItsOn(bool itsOn)
{
    mItsOn = itsOn;
    updateIcon();

    if(mItsOn)
    {
        emit tabToggled(static_cast<StalledIssueFilterCriterion>(mFilterCriterion));
    }

    setStyleSheet(styleSheet());
}

void StalledIssueTab::updateIcon()
{
    if(!mIconPrefix.isEmpty())
    {
        QString iconName =  QString::fromUtf8(":images/StalledIssues/") + mIconPrefix
                + (mItsOn ? QString::fromUtf8("-solid") : QString::fromUtf8("-outline"))
                + QString::fromUtf8(".png");
        QIcon icon = Utilities::getCachedPixmap(iconName);

        ui->icon->setPixmap(icon.pixmap(ui->icon->size()));
    }
}

void StalledIssueTab::toggleOffSiblings()
{
    QList<StalledIssueTab*> siblings = parentWidget()->findChildren<StalledIssueTab *>();

    foreach(auto& headerItem, siblings)
    {
        if(headerItem != this)
        {
            headerItem->setItsOn(false);
        }
    }
}

int StalledIssueTab::filterCriterion() const
{
    return mFilterCriterion;
}

void StalledIssueTab::setFilterCriterion(int filterCriterion)
{
    mFilterCriterion = filterCriterion;
    ui->title->setText(createTitle());
}

QString StalledIssueTab::createTitle()
{
    const auto itemsCount = MegaSyncApp->getStalledIssuesModel()->getCountByFilterCriterion(
        static_cast<StalledIssueFilterCriterion>(mFilterCriterion));
    switch (static_cast<StalledIssueFilterCriterion>(mFilterCriterion))
    {
        case StalledIssueFilterCriterion::ALL_ISSUES:
            return tr("All issues: %1").arg(itemsCount);
        case StalledIssueFilterCriterion::NAME_CONFLICTS:
            return tr("Name conflict: %n", "", itemsCount).arg(itemsCount);
        case StalledIssueFilterCriterion::ITEM_TYPE_CONFLICTS:
            return tr("Item type conflict: %n", "", itemsCount).arg(itemsCount);
        case StalledIssueFilterCriterion::OTHER_CONFLICTS:
            return tr("Other: %n", "", itemsCount).arg(itemsCount);
        case StalledIssueFilterCriterion::SOLVED_CONFLICTS:
            return tr("Resolved: %n", "", itemsCount).arg(itemsCount);
        default:
            return QString();
    }
}

void StalledIssueTab::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->title->setText(createTitle());
    }

    QFrame::changeEvent(event);
}
