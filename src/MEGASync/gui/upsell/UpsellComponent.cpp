#include "UpsellComponent.h"

#include "UpsellController.h"
#include "UpsellModel.h"

static bool qmlRegistrationDone = false;

UpsellComponent::UpsellComponent(QObject* parent):
    QMLComponent(parent),
    mController(std::make_shared<UpsellController>()),
    mModel(std::make_shared<UpsellModel>(mController))
{
    registerQmlModules();
    mController->setViewMode(UpsellPlans::ViewMode::TRANSFER_EXCEEDED);
}

QUrl UpsellComponent::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/upsell/UpsellDialog.qml"));
}

QString UpsellComponent::contextName()
{
    return QString::fromUtf8("upsellComponentAccess");
}

void UpsellComponent::registerQmlModules()
{
    if (!qmlRegistrationDone)
    {
        qmlRegisterUncreatableType<UpsellPlans>(
            "UpsellPlans",
            1,
            0,
            "UpsellPlans",
            QString::fromLatin1("UpsellPlans can only be used for the enum values"));
        qmlRegistrationDone = true;
    }
}

void UpsellComponent::buyButtonClicked()
{
    mController->openSelectedPlan();
}

void UpsellComponent::billedRadioButtonClicked(bool isMonthly)
{
    mController->setBilledPeriod(isMonthly);
}
