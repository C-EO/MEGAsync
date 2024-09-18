pragma Singleton
import QtQuick 2.15

QtObject {
    id: root

    readonly property string billedMonthly: qsTr("Billed monthly")
    readonly property string billedSaveUpText: qsTr("Save up to %%1%")
    readonly property string billedYearly: qsTr("Billed yearly")
    readonly property string buyPro: qsTr("Buy Pro")
    readonly property string estimatedPrice: qsTr("* Estimated price in your local currency. Your account will be billed in Euros for all transactions.")
    readonly property string notNow: qsTr("Not now")
    readonly property string storageAlmostFullTitle: qsTr("Your MEGA cloud storage is almost full")
    readonly property string storageAlmostFullText: qsTr("Upgrade your account to get more storage quota.
Or delete some files and [A]empty your rubbish bin[/A] to free up storage space.")
    readonly property string storageFullTitle: qsTr("Your MEGA cloud storage is full")
    readonly property string storageFullText: qsTr("Upgrade your account to get more storage quota.
Or delete some files and [A]empty your rubbish bin[/A] to free up storage space.")
    readonly property string transferQuotaExceededTitle: qsTr("Transfer quota exceeded")
    readonly property string transferQuotaExceededText: qsTr("You can’t continue downloading as you’ve used all of the transfer quota available to you.
Upgrade your account to get more transfer quota.
Or you can wait for [B]%1[/B] until more free quota becomes available for you.
[A]Learn more about transfer quota[/A].")

}
