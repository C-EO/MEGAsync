import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0
import components.texts 1.0 as Texts
import AccountStateQuickWidget 1.0

Item {
    id: root

    readonly property int defaultMargin: 12
    readonly property int cardSpacing: 48
    readonly property int smallSpacing: 4
    readonly property int compactSpacing: 8
    readonly property int dividerThickness: 1
    readonly property int titleTextPixelSize: 10
    readonly property int summaryTextPixelSize: 12

    readonly property int storageBannerPriority: {
        if (accountStateAccess.storageState === AccountStateQuickWidget.FULL) return 2
        if (accountStateAccess.storageState === AccountStateQuickWidget.WARNING) return 1
        return 0
    }
    readonly property int transferBannerPriority: {
        if (!accountStateAccess.showTransferCard || accountStateAccess.transferValueOnly) return 0
        if (accountStateAccess.transferState === AccountStateQuickWidget.FULL) return 2
        if (accountStateAccess.transferState === AccountStateQuickWidget.WARNING) return 1
        return 0
    }

    ColumnLayout {
        id: contentLayout

        anchors.fill: parent
        anchors.leftMargin: root.defaultMargin
        anchors.rightMargin: root.defaultMargin
        spacing: root.cardSpacing

        ColumnLayout {
            id: storageContent

            visible: accountStateAccess.showStorageCard
            Layout.fillWidth: true
            spacing: root.smallSpacing

            RowLayout {
                id: storageHeaderLayout

                Layout.fillWidth: true
                spacing: root.defaultMargin

                Texts.Text {
                    id: storageTitleText

                    Layout.fillWidth: true
                    text: SettingsStrings.storageSpace
                    font.pixelSize: root.titleTextPixelSize
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                }

                Texts.Text {
                    id: storageSummaryText

                    Layout.rightMargin: root.defaultMargin
                    text: accountStateAccess.storageSummary
                    font.pixelSize: root.summaryTextPixelSize
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignRight
                }
            }

            Rectangle {
                id: storageDivider

                Layout.fillWidth: true
                implicitHeight: root.dividerThickness
                color: ColorTheme.borderStrong
            }

            SettingsLinearProgress {
                id: storageProgress

                Layout.fillWidth: true
                Layout.topMargin: root.defaultMargin
                progressState: accountStateAccess.storageState
                segments: accountStateAccess.storageSegments
                bannerAlreadyShown: root.transferBannerPriority > root.storageBannerPriority
                onBannerActionClicked: accountStateAccess.upgradeRequested()
            }
        }

        ColumnLayout {
            id: transferContent

            visible: accountStateAccess.showTransferCard
            Layout.fillWidth: true
            spacing: root.smallSpacing

            RowLayout {
                id: transferHeaderLayout

                Layout.fillWidth: true
                spacing: root.defaultMargin

                Texts.Text {
                    id: transferTitleText

                    Layout.fillWidth: true
                    text: SettingsStrings.transferQuota
                    font.pixelSize: root.titleTextPixelSize
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                }

                Texts.Text {
                    id: transferSummaryText

                    Layout.rightMargin: root.defaultMargin
                    text: accountStateAccess.transferSummary
                    font.pixelSize: root.summaryTextPixelSize
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignRight
                }
            }

            Rectangle {
                id: transferDivider

                Layout.fillWidth: true
                implicitHeight: root.dividerThickness
                color:ColorTheme.borderStrong
            }

            SettingsLinearProgress {
                id: transferProgress
                Layout.fillWidth: true
                Layout.topMargin: root.defaultMargin
                visible: !accountStateAccess.transferValueOnly
                progressState: accountStateAccess.transferState
                segments: accountStateAccess.transferSegments
                bannerAlreadyShown: root.storageBannerPriority > 0
                                    && root.storageBannerPriority >= root.transferBannerPriority
                onBannerActionClicked: accountStateAccess.upgradeRequested()
            }

            Texts.Text {
                id: transferValueText

                Layout.fillWidth: true
                Layout.topMargin: root.compactSpacing
                visible: accountStateAccess.transferValueOnly
                text: accountStateAccess.transferValueText
                font.pixelSize: root.summaryTextPixelSize
                font.weight: Font.DemiBold
            }
        }

        Item {
            id: spacerItem

            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
