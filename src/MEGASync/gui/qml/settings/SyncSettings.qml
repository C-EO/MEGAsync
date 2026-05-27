import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.images 1.0
import components.texts 1.0
import components.buttons 1.0
import components.menus 1.0
import components.switch 1.0
import components.toolTips 1.0

import SyncSettingsModel 1.0

Item {
    id: root

    readonly property int defaultTopMargin: 19
    readonly property int syncTableSpacing: 4
    readonly property int titleTextPixelSize: 10
    readonly property int syncTablePadding: 12
    readonly property int syncNameLabelWidth: 412
    readonly property int listViewOfSyncItems: 270
    readonly property int syncItemBackgroundRadius: 6
    readonly property int syncItemBackgroundHeight: 32
    readonly property int syncItemHoritzontalPadding: 12
    readonly property int errorSyncItemHoritzontalPadding: 10
    readonly property int syncItemContentSpacing: 4
    readonly property int syncItemContentNameWidth: 404
    readonly property int folderSearchIconSize: 16
    readonly property int verticalAddSyncButtonSeparator: 16
    readonly property int leftPaddingAddSyncButton: 12
    readonly property int rightPaddingAddSyncButton: 16
    readonly property int maxSyncListSize: 390
    readonly property int issueLabelPixelSize: 12
    readonly property int issuePartHeigh: 58
    readonly property int issuePartSpacing: 4
    readonly property int switchSeparator: 12
    readonly property int noSyncsTopMargin: 128
    readonly property int megaDevicesImageSize: 120
    readonly property int titleNoSyncPixelSize: 20
    readonly property int descriptionNoSyncPixelSize: 16
    readonly property int noSyncUnderImageSpace: 24
    readonly property int noSyncTextsSpacing: 8
    readonly property int noSyncAboveButtonSpacing: 48
    readonly property int errorBorders: 2
    readonly property int backgroundColorAnimationTime: 200
    readonly property int toolTipShowDelay: 500
    readonly property int toolTipTimeoutToHide: 5000

    function getStatusDescription(status){
        switch(status) {
            case SyncSettingsModel.PENDING:
            case SyncSettingsModel.LOADING:
                    return SettingsStrings.syncStateLoading;
            case SyncSettingsModel.SUSPENDED:
                    return SettingsStrings.syncStatePaused;
            case SyncSettingsModel.FAIL:
                    return SettingsStrings.syncStateDisabled;
            case SyncSettingsModel.SCANNING:
                    return SettingsStrings.syncStateScanning;
            case SyncSettingsModel.SYNCING:
                    return SettingsStrings.syncStateSyncing;
            case SyncSettingsModel.SYNCED:
                    return SettingsStrings.syncStateSynced;
        }
    }

    ColumnLayout {
        id: noDataModelContentLayout

        visible: syncList.count === 0
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 0

        Item {
            Layout.preferredHeight: noSyncsTopMargin
            Layout.preferredWidth: parent.width
        }

        SvgImage {
            id: megaDevices

            source: Images.megaDevices
            sourceSize: Qt.size(megaDevicesImageSize, megaDevicesImageSize)
            Layout.alignment: Qt.AlignHCenter
        }

        Item {
            Layout.preferredHeight: noSyncUnderImageSpace
            Layout.preferredWidth: parent.width
        }

        Text {
            id: noSyncTitle

            text: SettingsStrings.titleNoSync
            font.pixelSize: root.titleNoSyncPixelSize
            font.weight: Font.DemiBold
            elide: Text.ElideRight
            Layout.alignment: Qt.AlignHCenter
        }

        Item {
            Layout.preferredHeight: noSyncTextsSpacing
            Layout.preferredWidth: parent.width
        }

        Text {
            id: noSyncDescription

            text: SettingsStrings.descriptionNoSync
            font.pixelSize: root.descriptionNoSyncPixelSize
            font.weight: Font.Normal
            elide: Text.ElideRight
            Layout.alignment: Qt.AlignHCenter
        }

        Item {
            Layout.preferredHeight: noSyncAboveButtonSpacing
            Layout.preferredWidth: parent.width
        }

        PrimaryButton {
            id: noSyncAddSyncButton

            text: SettingsStrings.addSync
            icons.source: Images.plus
            icons.position: Icon.Position.LEFT
            leftPadding: leftPaddingAddSyncButton
            rightPadding: rightPaddingAddSyncButton
            width: implicitWidth
            onClicked: {
                syncSettings.addSync();
            }
            Layout.alignment: Qt.AlignHCenter
        }
    }

    ColumnLayout {
        id: content

        anchors.fill: parent
        anchors.topMargin: defaultTopMargin
        spacing: syncTableSpacing
        visible: syncList.count > 0

        Row {
            id: syncsColumnsLabels

            Layout.preferredWidth: parent.width
            rightPadding: syncTablePadding
            leftPadding: rightPadding

            Text {
                id: syncNameColumn

                text: SettingsStrings.tableSyncsNameColumn
                font.pixelSize: root.titleTextPixelSize
                font.weight: Font.DemiBold
                elide: Text.ElideRight
                width: syncNameLabelWidth
            }

            Text {
                id: syncStatusColumn

                text: SettingsStrings.tableSyncsStatusColumn
                font.pixelSize: root.titleTextPixelSize
                font.weight: Font.DemiBold
                elide: Text.ElideRight
            }
        }

        Rectangle {
            id: syncsUnderLineTitle

            Layout.preferredWidth: parent.width
            Layout.preferredHeight: Constants.dividerThickness
            color: ColorTheme.borderStrong
        }

        ListView {
            id: syncList

            Layout.preferredHeight: Math.min(contentHeight, maxSyncListSize)
            Layout.minimumHeight: listViewOfSyncItems
            Layout.preferredWidth: parent.width
            model: syncSettingsModel
            delegate: syncComponent
            spacing: syncTableSpacing
            interactive: contentHeight > maxSyncListSize
            clip: true
            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }
        }

        Rectangle {
            id: syncsUnderLineTable

            Layout.preferredWidth: parent.width
            Layout.preferredHeight: Constants.dividerThickness
            color: ColorTheme.borderStrong
        }

        Item {
            Layout.preferredHeight: verticalAddSyncButtonSeparator
            Layout.preferredWidth: parent.width
        }

        Row {
            Layout.preferredWidth: parent.width
            layoutDirection: Qt.RightToLeft

            PrimaryButton {
                id: addSyncButton

                text: SettingsStrings.addSync
                icons.source: Images.plus
                icons.position: Icon.Position.LEFT
                leftPadding: leftPaddingAddSyncButton
                rightPadding: rightPaddingAddSyncButton
                width: implicitWidth
                onClicked: {
                    syncSettings.addSync();
                }
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width
        }

        RowLayout {
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: issuePartHeigh
            Layout.maximumHeight: issuePartHeigh
            spacing: 0

            ColumnLayout {
                spacing: issuePartSpacing

                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.leftMargin: syncItemHoritzontalPadding

                Text {
                    id: title

                    text: SettingsStrings.syncIssueTitle
                    font.pixelSize: issueLabelPixelSize
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                RichText {
                    id: description

                    manageMouse: true
                    manageHover: true
                    underlineLink: true
                    rawText: SettingsStrings.syncIssueDescription
                    font.pixelSize: issueLabelPixelSize
                    font.weight: Font.Normal
                    elide: Text.ElideRight
                    width: parent.width
                    url: serviceUrlsAccess.getSyncHelpUrl()
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }

            ColumnLayout {
                spacing: 0

                RowLayout {
                    spacing: 0

                    Item {
                        Layout.fillWidth: true
                        Layout.minimumWidth: switchSeparator
                    }

                    Switch {
                        id: fixIssue

                        checked: syncSettings.automaticSyncIssueResolverEnabled
                        onCheckedChanged: {
                            syncSettings.automaticSyncIssueResolverEnabled = checked;
                        }
                    }
                }

                Item {
                    Layout.fillHeight: true
                }
            }
        }
    }

    /*
      sync elements in the list.
    */
    Component {
        id: syncComponent

        Rectangle {
            id: syncItemBackground

            radius: syncItemBackgroundRadius
            height: content.implicitHeight
            width: syncList.width
            color: status === SyncSettingsModel.FAIL ? ColorTheme.notificationError : "transparent"

            function getSyncTextColor() {
                if (status === SyncSettingsModel.FAIL) {
                    return ColorTheme.textError;
                }
                else if (status === SyncSettingsModel.SUSPENDED) {
                    return ColorTheme.textSecondary;
                }
                else {
                    return ColorTheme.textPrimary;
                }
            }

            function getSyncIconColor() {
                if (status === SyncSettingsModel.FAIL) {
                    return ColorTheme.textError;
                }
                else if (status === SyncSettingsModel.SUSPENDED) {
                    return ColorTheme.iconSecondary;
                }
                else {
                    return ColorTheme.iconPrimary;
                }
            }


            ColumnLayout {
                id: content

                anchors.fill: parent
                spacing: root.errorBorders

                Rectangle {
                    id: syncItem

                    Layout.topMargin: status === SyncSettingsModel.FAIL ? root.errorBorders : 0
                    Layout.rightMargin: Layout.topMargin
                    Layout.leftMargin: Layout.topMargin
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true
                    Layout.preferredHeight: syncItemBackgroundHeight
                    color: syncItem.syncItemContainsMouse ? ColorTheme.surface1 : ColorTheme.pageBackground
                    radius: syncItemBackgroundRadius
                    property bool syncItemContainsMouse : syncItemMouseArea.containsMouse || folderSearchMouseArea.containsMouse || menuIconMouseArea.containsMouse || statusIconMouseArea.containsMouse || statusTextMouseArea.containsMouse

                    Behavior on color {
                        ColorAnimation {
                            duration: backgroundColorAnimationTime
                        }
                    }

                    function getStatusSyncIcon(status) {
                        switch(status) {
                            case SyncSettingsModel.FAIL:
                                return Images.alert_circle_small_thin_outline;

                            case SyncSettingsModel.SUSPENDED:
                                return Images.pause_thin_small_thin_outline;

                            default:
                                return Images.sync_01_small_thin_outline;
                        }
                    }

                    MouseArea {
                        id: syncItemMouseArea

                        anchors.fill: parent
                        hoverEnabled: true
                        propagateComposedEvents: true
                    }

                    RowLayout {
                        id: syncRow

                        anchors.fill: parent
                        anchors.leftMargin: (status === SyncSettingsModel.FAIL) ? errorSyncItemHoritzontalPadding : syncItemHoritzontalPadding
                        anchors.rightMargin: anchors.leftMargin
                        spacing: syncItemContentSpacing

                        Row {
                            id: syncNameContent

                            Layout.preferredHeight: parent.height
                            Layout.preferredWidth: syncItemContentNameWidth
                            width: syncItemContentNameWidth
                            spacing: syncItemContentSpacing

                            Text {
                                id: syncName

                                text: name
                                anchors.verticalCenter: parent.verticalCenter
                                color: getSyncTextColor()
                            }

                            SvgImage {
                                id: folderSearchIcon

                                color: getSyncIconColor()
                                source: Images.folder_search_small_thin_outline
                                sourceSize: Qt.size(folderSearchIconSize, folderSearchIconSize)
                                visible: syncItem.syncItemContainsMouse
                                anchors.verticalCenter: parent.verticalCenter

                                ToolTip {
                                    id: showInFolderTooltip

                                    visible: folderSearchMouseArea.containsMouse
                                    text: SettingsStrings.toolTipShowInFolder
                                    delay: toolTipShowDelay
                                    timeout: toolTipTimeoutToHide
                                }

                                MouseArea {
                                    id: folderSearchMouseArea

                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        syncSettings.exploreLocalSync(folder);
                                    }
                                }
                            }
                        }

                        RowLayout {
                            id: syncSatusContent

                            spacing: syncItemContentSpacing
                            Layout.fillWidth: true

                            SvgImage {
                                id: folderStatusIcon

                                color: statusIconMouseArea.containsMouse || statusTextMouseArea.containsMouse ? ColorTheme.iconPrimary : getSyncIconColor()
                                source: syncItem.getStatusSyncIcon(status)
                                sourceSize: Qt.size(folderSearchIconSize, folderSearchIconSize)

                                MouseArea {
                                    id: statusIconMouseArea

                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: status === SyncSettingsModel.SUSPENDED ? Qt.PointingHandCursor : Qt.ArrowCursor
                                    onClicked: {
                                        if (status === SyncSettingsModel.SUSPENDED) {
                                            syncSettings.resumeSync(index);
                                        }
                                    }
                                }
                            }

                            Text {
                                id: syncStatus

                                text: getStatusDescription(status)
                                color: statusTextMouseArea.containsMouse || statusIconMouseArea.containsMouse ? ColorTheme.textPrimary : getSyncTextColor()

                                MouseArea {
                                    id: statusTextMouseArea

                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: status === SyncSettingsModel.SUSPENDED ? Qt.PointingHandCursor : Qt.ArrowCursor
                                    onClicked: {
                                        if (status === SyncSettingsModel.SUSPENDED) {
                                            syncSettings.resumeSync(index);
                                        }
                                    }
                                }
                            }

                            Item {
                                Layout.fillWidth: true
                            }

                            SvgImage {
                                id: menuIcon

                                color: status === SyncSettingsModel.FAIL ? ColorTheme.textError : ColorTheme.iconPrimary
                                source: Images.threeDots
                                sourceSize: Qt.size(folderSearchIconSize, folderSearchIconSize)
                                visible: syncItem.syncItemContainsMouse

                                MouseArea {
                                    id: menuIconMouseArea

                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        menu.popup(syncItem.width - menu.width, syncItem.height)
                                    }
                                }
                            }
                        }
                    }

                    ContextMenu {
                        id: menu

                        onFocusChanged: {
                            if(menu.activeFocus === false){
                                menu.close();
                            }
                        }

                        ContextMenuItem {
                            text: SettingsStrings.menuActionsShowInFolder
                            icon.source: Images.folder_small_thin_outline
                            onTriggered: {
                                syncSettings.exploreLocalSync(folder);
                            }
                        }

                        ContextMenuItem {
                            text: SettingsStrings.menuActionsOpenInMega
                            icon.source: Images.mega_medium_thin_outline
                            onTriggered: {
                                syncSettings.openInMega(index);
                            }
                        }

                        MenuSeparator {
                        }

                        ContextMenuItem {
                            visible: status === SyncSettingsModel.SYNCED
                            height: visible ? implicitHeight : 0
                            text: SettingsStrings.menuActionsPause
                            icon.source: Images.pause_thin_small_thin_outline
                            onTriggered: {
                                syncSettings.pauseSync(index);
                            }
                        }

                        ContextMenuItem {
                            visible: status === SyncSettingsModel.SUSPENDED
                            height: visible ? implicitHeight : 0
                            text: SettingsStrings.menuActionsResume
                            icon.source: Images.play_small_thin_outline
                            onTriggered: {
                                syncSettings.resumeSync(index);
                            }
                        }

                        MenuSeparator {
                            visible: status === SyncSettingsModel.SYNCED || status === SyncSettingsModel.SUSPENDED
                            height: visible ? implicitHeight : 0
                        }

                        ContextMenuItem {
                            text: SettingsStrings.menuActionsManageExclusions
                            icon.source: Images.file_ignore_small_thin_outline
                            onTriggered: {
                                syncSettings.openExclusionsDialog(index);
                            }
                        }

                        MenuSeparator {
                        }

                        ContextMenuItem {
                            visible: status !== SyncSettingsModel.FAIL && status !== SyncSettingsModel.SUSPENDED
                            height: visible ? implicitHeight : 0
                            text: SettingsStrings.menuActionsRescan
                            icon.source: Images.search_large_small_thin_outline
                            onTriggered: {
                                syncSettings.rescan(index);
                            }
                        }

                        ContextMenuItem {
                            visible: status !== SyncSettingsModel.FAIL && status !== SyncSettingsModel.SUSPENDED
                            height: visible ? implicitHeight : 0
                            text: SettingsStrings.menuActionsReboot
                            icon.source: Images.rotate_cw_small_thin_outline
                            onTriggered: {
                                syncSettings.reboot(index);
                            }
                        }

                        MenuSeparator {
                            visible: status !== SyncSettingsModel.FAIL && status !== SyncSettingsModel.SUSPENDED
                            height: visible ? implicitHeight : 0
                        }

                        ContextMenuItem {
                            text: SettingsStrings.solveIssueRemoveSyncedFolder
                            textColor: ColorTheme.textError
                            imageColor: ColorTheme.textError
                            icon.source: Images.trash_small_thin_outline
                            onTriggered: {
                                syncSettings.remove(index);
                            }
                        }
                    }
                }

                Rectangle {
                    id: errorItem

                    visible: status === SyncSettingsModel.FAIL
                    Layout.bottomMargin: root.errorBorders
                    Layout.rightMargin: Layout.bottomMargin
                    Layout.leftMargin: Layout.bottomMargin
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true
                    Layout.preferredHeight: syncError.implicitHeight
                    color: ColorTheme.notificationError
                    radius: syncItemBackgroundRadius


                    SyncError {
                        id: syncError

                        errorId: error_id
                        errorMessage: error
                        syncLocalFolder: folder
                    }
                }
            }
        }
    }
}

