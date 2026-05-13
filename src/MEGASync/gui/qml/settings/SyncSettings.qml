import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.images 1.0
import components.texts 1.0
import components.buttons 1.0
import components.menus 1.0

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
    readonly property int switchButtonSeparator: 12

    readonly property int noSyncsTopMargin: 128
    readonly property int megaDevicesImageSize: 120
    readonly property int titleNoSyncPixelSize: 20
    readonly property int descriptionNoSyncPixelSize: 16
    readonly property int noSyncUnderImageSpace: 24
    readonly property int noSyncTextsSpacing: 8
    readonly property int noSyncAboveButtonSpacing: 48
    readonly property int errorBorders: 2
    readonly property int backgroundColorAnimationTime: 200
    readonly property int topErrorLabelMargin: 8
    readonly property int errorLabelMargin: 10

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
                id: syncName

                text: SettingsStrings.tableSyncsNameColumn
                font.pixelSize: root.titleTextPixelSize
                font.weight: Font.DemiBold
                elide: Text.ElideRight
                width: syncNameLabelWidth
            }

            Text {
                id: syncStatus

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

                    text: qsTr("Automatic sync issue resolution")
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
                    rawText: qsTr("MEGA automatically detects and resolves sync issues for you. Turn it off if you prefer to review and handle them manually. [A]Learn more[/A]")
                    font.pixelSize: issueLabelPixelSize
                    font.weight: Font.Normal
                    elide: Text.ElideRight
                    width: parent.width
                    url: serviceUrlsAccess.getCreateSyncHelpUrl()
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
                        Layout.minimumWidth: switchButtonSeparator
                    }

                    SwitchButton {
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
            color: statusid === SyncSettingsModel.ERROR ? ColorTheme.supportError : "transparent"

            ColumnLayout {
                id: content

                anchors.fill: parent
                spacing: root.errorBorders

                Rectangle {
                    id: syncItem

                    Layout.topMargin: statusid === SyncSettingsModel.ERROR ? root.errorBorders : 0
                    Layout.rightMargin: Layout.topMargin
                    Layout.leftMargin: Layout.topMargin
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true
                    Layout.preferredHeight: syncItemBackgroundHeight
                    color: statusid === SyncSettingsModel.ERROR ? ColorTheme.notificationError : syncItemContainsMouse ? ColorTheme.surface1 : ColorTheme.pageBackground
                    radius: syncItemBackgroundRadius
                    property bool syncItemContainsMouse : syncItemMouseArea.containsMouse || folderSearchMouseArea.containsMouse || menuIconMouseArea.containsMouse

                    Behavior on color {
                        ColorAnimation {
                            duration: backgroundColorAnimationTime
                        }
                    }

                    function getStatusSyncIcon(statusid)
                    {
                        switch(statusid)
                        {
                            case SyncSettingsModel.ERROR:
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
                        anchors.leftMargin: (statusid === SyncSettingsModel.ERROR) ? errorSyncItemHoritzontalPadding : syncItemHoritzontalPadding
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
                                color: statusid === SyncSettingsModel.ERROR ? ColorTheme.textError : enabled ? ColorTheme.textPrimary : ColorTheme.textDisabled
                            }

                            SvgImage {
                                id: folderSearchIcon

                                color: statusid === SyncSettingsModel.ERROR ? ColorTheme.textError : ColorTheme.iconPrimary
                                source: Images.folder_search_small_thin_outline
                                sourceSize: Qt.size(folderSearchIconSize, folderSearchIconSize)
                                visible: syncItem.syncItemContainsMouse
                                anchors.verticalCenter: parent.verticalCenter

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

                                color: statusid === SyncSettingsModel.ERROR ? ColorTheme.textError : ColorTheme.iconPrimary
                                source: syncItem.getStatusSyncIcon(statusid)
                                sourceSize: Qt.size(folderSearchIconSize, folderSearchIconSize)
                            }

                            Text {
                                id: syncStatus

                                text: status
                                color: statusid === SyncSettingsModel.ERROR ? ColorTheme.textError : enabled ? ColorTheme.textPrimary : ColorTheme.textDisabled
                            }

                            Item {
                                Layout.fillWidth: true
                            }

                            SvgImage {
                                id: menuIcon

                                color: statusid === SyncSettingsModel.ERROR ? ColorTheme.textError : ColorTheme.iconPrimary
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
                            visible: statusid === SyncSettingsModel.ERROR
                            height: visible ? implicitHeight : 0
                            text: "Solve issues"
                            icon.source: Images.lightbulb_small_thin_outline
                            onTriggered: {
                            }
                        }

                        MenuSeparator {
                            visible: statusid === SyncSettingsModel.ERROR
                            height: visible ? implicitHeight : 0
                        }

                        ContextMenuItem {
                            text: "Show in folder"
                            icon.source: Images.folder_small_thin_outline
                            onTriggered: {
                                syncSettings.exploreLocalSync(folder);
                            }
                        }

                        ContextMenuItem {
                            text: "Open in mega"
                            icon.source: Images.mega_medium_thin_outline
                            onTriggered: {
                                syncSettings.openInMega(index);
                            }
                        }

                        MenuSeparator {
                        }

                        ContextMenuItem {
                            visible: statusid === SyncSettingsModel.RUNNING
                            height: visible ? implicitHeight : 0
                            text: "Pause"
                            icon.source: Images.pause_thin_small_thin_outline
                            onTriggered: {
                                syncSettings.pauseSync(index);
                            }
                        }

                        ContextMenuItem {
                            visible: statusid === SyncSettingsModel.SUSPENDED
                            height: visible ? implicitHeight : 0
                            text: "Resume"
                            icon.source: Images.play_small_thin_outline
                            onTriggered: {
                                syncSettings.resumeSync(index);
                            }
                        }

                        MenuSeparator {
                            visible: statusid === SyncSettingsModel.RUNNING || statusid === SyncSettingsModel.SUSPENDED
                            height: visible ? implicitHeight : 0
                        }

                        ContextMenuItem {
                            text: "Manage exlusions"
                            icon.source: Images.file_ignore_small_thin_outline
                            onTriggered: {
                                syncSettings.openExclusionsDialog(index);
                            }
                        }

                        MenuSeparator {
                        }

                        ContextMenuItem {
                            visible: statusid !== SyncSettingsModel.ERROR && statusid !== SyncSettingsModel.SUSPENDED
                            height: visible ? implicitHeight : 0
                            text: "Rescan"
                            icon.source: Images.search_large_small_thin_outline
                            onTriggered: {
                                syncSettings.rescan(index);
                            }
                        }

                        ContextMenuItem {
                            visible: statusid !== SyncSettingsModel.ERROR && statusid !== SyncSettingsModel.SUSPENDED
                            height: visible ? implicitHeight : 0
                            text: "Reboot"
                            icon.source: Images.rotate_cw_small_thin_outline
                            onTriggered: {
                                syncSettings.reboot(index);
                            }
                        }

                        MenuSeparator {
                            visible: statusid !== SyncSettingsModel.ERROR && statusid !== SyncSettingsModel.SUSPENDED
                            height: visible ? implicitHeight : 0
                        }

                        ContextMenuItem {
                            text: "Remove synced folder"
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

                    visible: statusid === SyncSettingsModel.ERROR
                    Layout.bottomMargin: root.errorBorders
                    Layout.rightMargin: Layout.bottomMargin
                    Layout.leftMargin: Layout.bottomMargin
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true
                    Layout.preferredHeight: errorInfo.implicitHeight
                    color: ColorTheme.notificationError
                    radius: syncItemBackgroundRadius

                    ColumnLayout {
                        id: errorInfo

                        anchors.fill: parent
                        spacing: 4

                        Text {
                            id: errorLabel

                            Layout.topMargin: root.topErrorLabelMargin
                            Layout.rightMargin: root.errorLabelMargin
                            Layout.leftMargin: root.errorLabelMargin
                            Layout.fillWidth: true

                            text: error
                            font.pixelSize: issueLabelPixelSize
                            font.weight: Font.Normal
                            elide: Text.ElideRight
                            color: ColorTheme.textError
                        }

                        PrimaryButton {
                            id: action

                            Layout.bottomMargin: root.errorLabelMargin
                            Layout.topMargin: root.topErrorLabelMargin
                            Layout.rightMargin: root.errorLabelMargin
                            Layout.leftMargin: root.errorLabelMargin

                            sizes: SmallSizes {}
                            icons.source: Images.pen_2_small_thin_outline
                            icons.position: Icon.Position.LEFT

                            text: qsTr("Edit sync")
                        }
                    }
                }
            }
        }
    }
}

