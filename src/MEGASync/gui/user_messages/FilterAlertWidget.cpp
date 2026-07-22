#include "FilterAlertWidget.h"

#include "ui_FilterAlertWidget.h"

#include <QStyle>

namespace
{
constexpr int RED_BUBBLE_MARGIN = 17;
}

FilterAlertWidget::FilterAlertWidget(QWidget* parent)
    : QWidget(parent)
    , mUi(new Ui::FilterAlertWidget)
    , mCurrentFilter(MessageType::ALL)
{
    mUi->setupUi(this);
    setUnseenNotifications(0, 0, 0, 0);

    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Popup);
    setAttribute(Qt::WA_TranslucentBackground);
    adjustSize();
    prepareEventFilters();
}

FilterAlertWidget::~FilterAlertWidget()
{
    delete mUi;
}

void FilterAlertWidget::reset()
{
    setUnseenNotifications(0, 0, 0, 0);
}

MessageType FilterAlertWidget::getCurrentFilter() const
{
    return mCurrentFilter;
}

void FilterAlertWidget::setUnseenNotifications(long long all, long long contacts, long long shares, long long payment)
{
    long long allUnseen = all > 0 ? all : 0;
    long long contactsUnseen = contacts > 0 ? contacts : 0;
    long long sharesUnseen = shares > 0 ? shares : 0;
    long long paymentUnseen = payment > 0 ? payment : 0;

    if (!allUnseen)
    {
        mUi->bNumberNotifications->hide();
    }
    else
    {
        mUi->bNumberNotifications->setText(QString::number(allUnseen));
        mUi->bNumberNotifications->setFixedWidth(mUi->bNumberNotifications->fontMetrics().horizontalAdvance(QString::number(allUnseen)) + RED_BUBBLE_MARGIN);
        mUi->bNumberNotifications->show();
    }

    if (!contactsUnseen)
    {
        mUi->bNumberNotificationsContacts->hide();
    }
    else
    {
        mUi->bNumberNotificationsContacts->setText(QString::number(contactsUnseen));
        mUi->bNumberNotificationsContacts->setFixedWidth(mUi->bNumberNotificationsContacts->fontMetrics().horizontalAdvance(QString::number(contactsUnseen)) + RED_BUBBLE_MARGIN);
        mUi->bNumberNotificationsContacts->show();
    }

    if (!sharesUnseen)
    {
        mUi->bNumberNotificationsShares->hide();
    }
    else
    {
        mUi->bNumberNotificationsShares->setText(QString::number(sharesUnseen));
        mUi->bNumberNotificationsShares->setFixedWidth(mUi->bNumberNotificationsShares->fontMetrics().horizontalAdvance(QString::number(sharesUnseen)) + RED_BUBBLE_MARGIN);
        mUi->bNumberNotificationsShares->show();
    }

    if (!paymentUnseen)
    {
        mUi->bNumberNotificationsPayments->hide();
    }
    else
    {
        mUi->bNumberNotificationsPayments->setText(QString::number(paymentUnseen));
        mUi->bNumberNotificationsPayments->setFixedWidth(mUi->bNumberNotificationsPayments->fontMetrics().horizontalAdvance(QString::number(paymentUnseen)) + RED_BUBBLE_MARGIN);
        mUi->bNumberNotificationsPayments->show();
    }
    adjustSize();
}

void FilterAlertWidget::on_bAll_clicked()
{
    mCurrentFilter = MessageType::ALL;
    emit filterClicked(mCurrentFilter);
}

void FilterAlertWidget::onBContactsClicked()
{
    mCurrentFilter = MessageType::ALERT_CONTACTS;
    emit filterClicked(mCurrentFilter);
}

void FilterAlertWidget::onBSharesClicked()
{
    mCurrentFilter = MessageType::ALERT_SHARES;
    emit filterClicked(mCurrentFilter);
}

void FilterAlertWidget::onBPaymentClicked()
{
    mCurrentFilter = MessageType::ALERT_PAYMENTS;
    emit filterClicked(mCurrentFilter);
}

bool FilterAlertWidget::event(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        mUi->retranslateUi(this);
    }
    return QWidget::event(event);
}

void FilterAlertWidget::hideEvent(QHideEvent* event)
{
    // The popup is dismissed on every selection. Qt drives the QSS :hover background off each
    // widget's WA_UnderMouse flag, which is only updated during real mouse traversal: when the
    // popup hides with the cursor still over a row, the flag stays set and that row (and its
    // inner .menu-style text button) is shown highlighted the next time the popup appears.
    // Clear the flag explicitly on every dismissal so the popup always reopens clean.
    const auto widgets = findChildren<QWidget*>();
    for (QWidget* widget: widgets)
    {
        if (!widget->testAttribute(Qt::WA_UnderMouse))
        {
            continue;
        }
        widget->setAttribute(Qt::WA_UnderMouse, false);
        widget->setProperty("active", false);
        widget->style()->unpolish(widget);
        widget->style()->polish(widget);
    }
    update();
    QWidget::hideEvent(event);
}

bool FilterAlertWidget::eventFilter(QObject* obj, QEvent* event)
{
    // We're only interested in mouse press/release events
    if (event->type() != QEvent::MouseButtonPress && event->type() != QEvent::MouseButtonRelease)
    {
        return QObject::eventFilter(obj, event);
    }

    QWidget* widget = qobject_cast<QWidget*>(obj);
    if (!widget)
    {
        return QObject::eventFilter(obj, event);
    }

    if (!(widget == mUi->wContactsNotifications || widget == mUi->wSharesNotifications ||
          widget == mUi->wPaymentNotifications || widget == mUi->wAllNotifications))
    {
        widget = widget->parentWidget();
    }

    if (!widget)
    {
        return QObject::eventFilter(obj, event);
    }

    if (event->type() == QEvent::MouseButtonPress)
    {
        widget->setProperty("active", true);
        widget->style()->unpolish(widget);
        widget->style()->polish(widget);
    }
    else if (event->type() == QEvent::MouseButtonRelease)
    {
        widget->setProperty("active", false);
        widget->style()->unpolish(widget);
        widget->style()->polish(widget);

        if (widget == mUi->wContactsNotifications)
        {
            onBContactsClicked();
        }
        else if (widget == mUi->wSharesNotifications)
        {
            onBSharesClicked();
        }
        else if (widget == mUi->wPaymentNotifications)
        {
            onBPaymentClicked();
        }
    }
    return QObject::eventFilter(obj, event);
}

void FilterAlertWidget::prepareEventFilters()
{
    auto initializeEventFilter = [this](QWidget* widget)
    {
        widget->installEventFilter(this);
        for (auto child: widget->children())
        {
            child->installEventFilter(this);
        }
    };
    initializeEventFilter(mUi->wContactsNotifications);
    initializeEventFilter(mUi->wSharesNotifications);
    initializeEventFilter(mUi->wPaymentNotifications);
    initializeEventFilter(mUi->wAllNotifications);
}
