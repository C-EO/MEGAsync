import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.images 1.0
import components.texts 1.0
import components.buttons 1.0
import components.menus 1.0
import components.toolTips 1.0
import components.busyIndicator 1.0

import BackupSettingsModel 1.0

Item {
    id: root

    readonly property int defaultTopMargin: 24
    readonly property int backupTableSpacing: 4
    readonly property int titleTextPixelSize: 10
    readonly property int backupTablePadding: 12
    readonly property int backupStatusLabelWidth: 156
    readonly property int listViewOfBackupItems: 270
    readonly property int backupItemBackgroundRadius: 6
    readonly property int errorBackupItemRadius: 4
    readonly property int backupItemBackgroundHeight: 32
    readonly property int backupItemHorizontalPadding: 12
    readonly property int errorBackupItemHorizontalPadding: 10
    readonly property int backupItemContentSpacing: 4
    readonly property int backupItemContentStatusWidth: 172
    readonly property int folderSearchIconSize: 16
    readonly property int verticalAddBackupButtonSeparator: 16
    readonly property int leftPaddingAddBackupButton: 12
    readonly property int rightPaddingAddBackupButton: 16
    readonly property int backupFooterHorizontalMargin: 12
    readonly property int backupFooterBottomMargin: 12
    readonly property int backupFooterLabelSpacing: 4
    readonly property int backupFooterSpacing: 8
    readonly property int backupFooterFieldHeight: 36
    readonly property int backupFooterFieldRadius: 8
    readonly property int backupFooterFieldHorizontalPadding: 10
    readonly property int backupFooterFieldSpacing: 6
    readonly property int backupFooterFieldTextPixelSize: 14
    readonly property int maxBackupListSize: 390
    readonly property int issueLabelPixelSize: 12
    readonly property int noBackupsTopMargin: 128
    readonly property int backupImageSize: 120
    readonly property int titleNoBackupPixelSize: 20
    readonly property int descriptionNoBackupPixelSize: 16
    readonly property int noBackupUnderImageSpace: 24
    readonly property int noBackupTextsSpacing: 8
    readonly property int noBackupAboveButtonSpacing: 48
    readonly property int errorBorders: 2
    readonly property int backgroundColorAnimationTime: 200
    readonly property int toolTipShowDelay: 500
    readonly property int toolTipTimeoutToHide: 5000
    readonly property size iconOrderFlagSize: Qt.size(12, 12)

    function getStatusDescription(status) {
        switch(status) {
            case BackupSettingsModel.PENDING:
            case BackupSettingsModel.LOADING:
                return SettingsStrings.backupStateLoading;
            case BackupSettingsModel.SUSPENDED:
                return SettingsStrings.backupStatePaused;
            case BackupSettingsModel.FAIL:
                return SettingsStrings.backupStateDisabled;
            case BackupSettingsModel.SCANNING:
                return SettingsStrings.backupStateScanning;
            case BackupSettingsModel.BACKING_UP:
                return SettingsStrings.backupStateBackingUp;
            case BackupSettingsModel.BACKED_UP:
                return SettingsStrings.backupStateBackedUp;
        }
    }

    function getShowInFolderText() {
        if (OS.isMac()) {
            return SettingsStrings.menuActionsShowInFinder;
        }
        else if (OS.isWindows()) {
            return SettingsStrings.menuActionsShowInFileExplorer;
        }
        else {
            return SettingsStrings.menuActionsShowInFolder;
        }
    }

    ColumnLayout {
        id: noDataModelContentLayout

        visible: backupList.count === 0
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 0

        Item {
            Layout.preferredHeight: noBackupsTopMargin
            Layout.preferredWidth: parent.width
        }

        SvgImage {
            id: backupImage

            source: Images.backupDevices
            sourceSize: Qt.size(backupImageSize, backupImageSize)
            Layout.alignment: Qt.AlignHCenter
        }

        Item {
            Layout.preferredHeight: noBackupUnderImageSpace
            Layout.preferredWidth: parent.width
        }

        Text {
            id: noBackupTitle

            text: SettingsStrings.titleNoBackup
            font.pixelSize: root.titleNoBackupPixelSize
            font.weight: Font.DemiBold
            elide: Text.ElideRight
            Layout.alignment: Qt.AlignHCenter
        }

        Item {
            Layout.preferredHeight: noBackupTextsSpacing
            Layout.preferredWidth: parent.width
        }

        Text {
            id: noBackupDescription

            text: SettingsStrings.descriptionNoBackup
            font.pixelSize: root.descriptionNoBackupPixelSize
            font.weight: Font.Normal
            elide: Text.ElideRight
            Layout.alignment: Qt.AlignHCenter
        }

        Item {
            Layout.preferredHeight: noBackupAboveButtonSpacing
            Layout.preferredWidth: parent.width
        }

        PrimaryButton {
            id: noBackupAddButton

            text: SettingsStrings.addBackup
            icons.source: Images.plus
            icons.position: Icon.Position.LEFT
            leftPadding: leftPaddingAddBackupButton
            rightPadding: rightPaddingAddBackupButton
            width: implicitWidth
            onClicked: {
                backupSettings.addBackup();
            }
            Layout.alignment: Qt.AlignHCenter
        }
    }

    ColumnLayout {
        id: content

        anchors.fill: parent
        anchors.topMargin: defaultTopMargin
        spacing: backupTableSpacing
        visible: backupList.count > 0

        RowLayout {
            id: backupsColumnsLabels

            Layout.preferredWidth: parent.width
            Layout.rightMargin: backupTablePadding
            Layout.leftMargin: backupTablePadding
            spacing: 0

            Text {
                id: backupNameColumn

                property bool sortByNameAscending: true
                text: SettingsStrings.tableBackupsNameColumn
                color: ColorTheme.textAccent
                font.pixelSize: root.titleTextPixelSize
                font.weight: Font.DemiBold
                elide: Text.ElideRight

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        backupNameColumn.sortByNameAscending = !backupNameColumn.sortByNameAscending
                        backupSettings.sortModelByName(backupNameColumn.sortByNameAscending)
                        backupNameColumnOrderSymbol.visible = true
                        backupStatusColumnOrderSymbol.visible = false
                    }
                }
            }

            Item {
                id: nameOrderIconSpacer
                Layout.preferredWidth: root.backupItemContentSpacing
            }

            SvgImage {
                id: backupNameColumnOrderSymbol

                source: backupNameColumn.sortByNameAscending ? Images.arrow_up_medium_regular_outline : Images.arrow_down_medium_regular_outline
                visible: false
                sourceSize: iconOrderFlagSize
                color: ColorTheme.iconPrimary
            }

            Item {
                Layout.fillWidth: true
            }

            RowLayout {
                id: backupStatusColumnSpace

                Layout.preferredHeight: parent.height
                Layout.preferredWidth: backupStatusLabelWidth
                Layout.maximumWidth: backupStatusLabelWidth
                Layout.alignment: Qt.AlignVCenter
                spacing: 0

                Text {
                    id: backupStatusColumn

                    property bool sortByStatusAscending: true
                    text: SettingsStrings.tableBackupsStatusColumn
                    color: ColorTheme.textAccent
                    font.pixelSize: root.titleTextPixelSize
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            backupStatusColumn.sortByStatusAscending = !backupStatusColumn.sortByStatusAscending
                            backupSettings.sortModelByStatus(backupStatusColumn.sortByStatusAscending)
                            backupNameColumnOrderSymbol.visible = false
                            backupStatusColumnOrderSymbol.visible = true
                        }
                    }
                }

                Item {
                    id: statusOrderIconSpacer
                    Layout.preferredWidth: root.backupItemContentSpacing
                    Layout.maximumWidth: root.backupItemContentSpacing
                }

                SvgImage {
                    id: backupStatusColumnOrderSymbol

                    source: backupStatusColumn.sortByStatusAscending ? Images.arrow_up_medium_regular_outline : Images.arrow_down_medium_regular_outline
                    visible: false
                    color: ColorTheme.iconPrimary
                    sourceSize: iconOrderFlagSize
                }

                Item {
                    Layout.fillWidth: true
                }
            }
        }

        Rectangle {
            id: backupsUnderLineTitle

            Layout.preferredWidth: parent.width
            Layout.preferredHeight: Constants.dividerThickness
            color: ColorTheme.borderStrong
        }

        ListView {
            id: backupList

            Layout.preferredWidth: parent.width
            Layout.minimumHeight: listViewOfBackupItems
            Layout.fillHeight: true
            Layout.maximumHeight: Math.max(listViewOfBackupItems, contentHeight)

            model: backupSettingsModel
            delegate: backupComponent
            spacing: backupTableSpacing
            interactive: contentHeight > height
            clip: true

            ScrollBar.vertical: ScrollBar {
                policy: backupList.contentHeight > backupList.height
                      ? ScrollBar.AlwaysOn
                      : ScrollBar.AlwaysOff
            }
        }

        Rectangle {
            id: backupsUnderLineTable

            Layout.preferredWidth: parent.width
            Layout.preferredHeight: Constants.dividerThickness
            color: ColorTheme.borderStrong
        }

        Item {
            Layout.preferredHeight: verticalAddBackupButtonSeparator
            Layout.preferredWidth: parent.width
        }

        Row {
            Layout.preferredWidth: parent.width
            layoutDirection: Qt.RightToLeft

            PrimaryButton {
                id: addBackupButton

                text: SettingsStrings.addBackup
                icons.source: Images.plus
                icons.position: Icon.Position.LEFT
                leftPadding: leftPaddingAddBackupButton
                rightPadding: rightPaddingAddBackupButton
                width: implicitWidth
                onClicked: {
                    backupSettings.addBackup();
                }
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width
        }

        ColumnLayout {
            id: backupFolderFooter

            Layout.fillWidth: true
            Layout.leftMargin: backupFooterHorizontalMargin
            Layout.rightMargin: backupFooterHorizontalMargin
            Layout.bottomMargin: backupFooterBottomMargin
            spacing: backupFooterLabelSpacing

            Text {
                id: backupFolderLabel

                text: SettingsStrings.backupFolderLabel
                color: ColorTheme.textPrimary
                font.pixelSize: root.issueLabelPixelSize
                font.weight: Font.DemiBold
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            RowLayout {
                id: backupFolderControls

                Layout.fillWidth: true
                spacing: backupFooterSpacing

                Rectangle {
                    id: backupFolderField

                    Layout.fillWidth: true
                    Layout.preferredHeight: backupFooterFieldHeight
                    Layout.alignment: Qt.AlignVCenter
                    color: ColorTheme.pageBackground
                    border.color: ColorTheme.borderStrongSelected
                    border.width: Constants.dividerThickness
                    radius: backupFooterFieldRadius

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: backupFooterFieldHorizontalPadding
                        anchors.rightMargin: backupFooterFieldHorizontalPadding
                        spacing: backupFooterFieldSpacing

                        SvgImage {
                            id: backupFolderIcon

                            source: Images.database_small_thin_outline
                            sourceSize: Qt.size(folderSearchIconSize, folderSearchIconSize)
                            color: ColorTheme.iconPrimary
                            Layout.alignment: Qt.AlignVCenter
                        }

                        Text {
                            id: backupFolderPath

                            text: backupSettings.backupFolderPath
                            color: ColorTheme.textPrimary
                            font.pixelSize: root.backupFooterFieldTextPixelSize
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignVCenter
                        }
                    }
                }

                SecondaryButton {
                    id: viewBackupFolderButton

                    text: SettingsStrings.viewInMega
                    enabled: backupSettings.backupFolderAvailable
                    Layout.alignment: Qt.AlignVCenter
                    onClicked: {
                        backupSettings.openBackupFolder();
                    }
                }
            }
        }
    }

    /*
      Backup elements in the list.
    */
    Component {
        id: backupComponent

        Rectangle {
            id: backupItemBackground

            radius: backupItemBackgroundRadius
            height: backupContent.implicitHeight
            width: backupList.width
            color: status === BackupSettingsModel.FAIL ? ColorTheme.notificationError : "transparent"

            function getBackupTextColor() {
                if (status === BackupSettingsModel.FAIL) {
                    return ColorTheme.textError;
                }
                else if (status === BackupSettingsModel.SUSPENDED) {
                    return ColorTheme.textSecondary;
                }
                else {
                    return ColorTheme.textPrimary;
                }
            }

            function getBackupIconColor() {
                if (status === BackupSettingsModel.FAIL) {
                    return ColorTheme.textError;
                }
                else if (status === BackupSettingsModel.SUSPENDED) {
                    return ColorTheme.iconSecondary;
                }
                else {
                    return ColorTheme.iconPrimary;
                }
            }

            function getBackupStatusTextColor() {
                if (status === BackupSettingsModel.FAIL) {
                    return ColorTheme.textError;
                }
                else {
                    return ColorTheme.textSecondary;
                }
            }

            function getBackupStatusIconColor() {
                if (status === BackupSettingsModel.FAIL) {
                    return ColorTheme.textError;
                }
                else {
                    return ColorTheme.iconSecondary;
                }
            }

            ColumnLayout {
                id: backupContent

                anchors.fill: parent
                spacing: root.errorBorders

                Rectangle {
                    id: backupItem

                    Layout.topMargin: status === BackupSettingsModel.FAIL ? root.errorBorders : 0
                    Layout.rightMargin: Layout.topMargin
                    Layout.leftMargin: Layout.topMargin
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true
                    Layout.preferredHeight: backupItemBackgroundHeight
                    color: backupItem.backupItemContainsMouse || menu.visible ? ColorTheme.surface1 : ColorTheme.pageBackground
                    radius: status === BackupSettingsModel.FAIL ? errorBackupItemRadius : backupItemBackgroundRadius
                    property bool backupItemContainsMouse: backupItemMouseArea.containsMouse || nameAndFolderSearchMouseArea.containsMouse ||
                                                          menuButton.hovered || statusBackupMouseArea.containsMouse ||
                                                          nameMouseArea.containsMouse

                    Behavior on color {
                        ColorAnimation {
                            duration: backgroundColorAnimationTime
                        }
                    }

                    function getStatusBackupIcon(status) {
                        switch(status) {
                            case BackupSettingsModel.FAIL:
                                return Images.alert_circle_small_thin_outline;

                            case BackupSettingsModel.SUSPENDED:
                                return Images.pause_small_thin_outline;

                            default:
                                return Images.database_small_thin_outline;
                        }
                    }

                    MouseArea {
                        id: backupItemMouseArea

                        anchors.fill: parent
                        hoverEnabled: true
                        propagateComposedEvents: true
                        acceptedButtons: Qt.RightButton

                        onClicked: function(mouse) {
                            if (mouse.button === Qt.RightButton) {
                                menu.popup(mouse.x, mouse.y)
                            }
                        }
                    }

                    RowLayout {
                        id: backupRow

                        anchors.fill: parent
                        anchors.leftMargin: (status === BackupSettingsModel.FAIL) ? errorBackupItemHorizontalPadding : backupItemHorizontalPadding
                        anchors.rightMargin: 0
                        spacing: backupItemContentSpacing

                        Item {
                            id: backupParentNameContent

                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignVCenter

                            RowLayout {
                                id: backupNameContent

                                anchors.fill: parent

                                RowLayout {
                                    id: backupNameWrapper

                                    Layout.alignment: Qt.AlignVCenter
                                    Layout.preferredWidth: backupNameWrapper.implicitWidth
                                    Layout.preferredHeight: parent.height

                                    spacing: backupItemContentSpacing

                                    Text {
                                        id: backupName

                                        text: name
                                        Layout.alignment: Qt.AlignVCenter
                                        wrapMode: Text.NoWrap
                                        elide: Text.ElideRight
                                        Layout.preferredWidth: Math.min(implicitWidth, backupNameContent.width - folderSearchIcon.width - 2 * backupItemContentSpacing)
                                        color: nameAndFolderSearchMouseArea.containsMouse && status === BackupSettingsModel.SUSPENDED ? ColorTheme.textPrimary : backupItemBackground.getBackupTextColor()

                                        ToolTip {
                                            id: fullNameTooltip

                                            visible: nameMouseArea.containsMouse && (backupName.implicitWidth > (backupNameContent.width - folderSearchIcon.width - 2 * backupItemContentSpacing))
                                            text: name
                                            delay: root.toolTipShowDelay
                                            maxWidth: backupNameContent.width - folderSearchIcon.width - 2 * backupItemContentSpacing
                                        }

                                        MouseArea {
                                            id: nameMouseArea

                                            anchors.fill: parent
                                            hoverEnabled: true
                                        }
                                    }

                                    SvgImage {
                                        id: folderSearchIcon

                                        color: nameAndFolderSearchMouseArea.containsMouse && status === BackupSettingsModel.SUSPENDED ? ColorTheme.iconPrimary : backupItemBackground.getBackupIconColor()
                                        source: Images.folder_search_small_thin_outline
                                        sourceSize: Qt.size(folderSearchIconSize, folderSearchIconSize)
                                        visible: backupItem.backupItemContainsMouse
                                        Layout.alignment: Qt.AlignVCenter

                                        ToolTip {
                                            id: showInFolderTooltip

                                            visible: nameAndFolderSearchMouseArea.containsMouse
                                            text: SettingsStrings.toolTipShowInFolder
                                            delay: root.toolTipShowDelay
                                        }

                                        MouseArea {
                                            id: nameAndFolderSearchMouseArea

                                            anchors.fill: parent
                                            hoverEnabled: true
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: {
                                                backupSettings.exploreLocalBackup(folder);
                                            }
                                        }
                                    }
                                }

                                Item {
                                    id: nameHorizontalSpacer
                                    Layout.fillWidth: true
                                }
                            }
                        }

                        RowLayout {
                            id: backupStatusContent

                            spacing: 0
                            Layout.minimumWidth: backupItemContentStatusWidth
                            Layout.maximumWidth: backupItemContentStatusWidth
                            Layout.preferredWidth: backupItemContentStatusWidth
                            Layout.preferredHeight: parent.height

                            Item {
                                id: backupStatusWrapper

                                Layout.alignment: Qt.AlignVCenter
                                Layout.fillWidth: true
                                Layout.preferredHeight: parent.height

                                RowLayout {
                                    id: backupStatusRowContent

                                    anchors.verticalCenter: parent.verticalCenter

                                    BusyIndicator {
                                        id: folderStatusBusyIndicator

                                        Layout.alignment: Qt.AlignVCenter
                                        imageSize: Qt.size(folderSearchIconSize, folderSearchIconSize)
                                        color: backupItemBackground.getBackupStatusIconColor()
                                        visible: status === BackupSettingsModel.PENDING
                                                 || status === BackupSettingsModel.LOADING
                                                 || status === BackupSettingsModel.SCANNING
                                                 || status === BackupSettingsModel.BACKING_UP
                                    }

                                    SvgImage {
                                        id: folderStatusIcon

                                        Layout.alignment: Qt.AlignVCenter
                                        color: statusBackupMouseArea.containsMouse && status === BackupSettingsModel.SUSPENDED ? ColorTheme.iconPrimary : backupItemBackground.getBackupStatusIconColor()
                                        source: backupItem.getStatusBackupIcon(status)
                                        sourceSize: Qt.size(folderSearchIconSize, folderSearchIconSize)
                                        visible: !folderStatusBusyIndicator.visible
                                    }

                                    Text {
                                        id: backupStatus

                                        Layout.alignment: Qt.AlignVCenter
                                        text: getStatusDescription(status)
                                        color: statusBackupMouseArea.containsMouse && status === BackupSettingsModel.SUSPENDED ? ColorTheme.textPrimary : backupItemBackground.getBackupStatusTextColor()
                                    }
                                }

                                MouseArea {
                                    id: statusBackupMouseArea

                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: status === BackupSettingsModel.SUSPENDED ? Qt.PointingHandCursor : Qt.ArrowCursor
                                    onClicked: {
                                        if (status === BackupSettingsModel.SUSPENDED) {
                                            backupSettings.resumeBackup(index);
                                        }
                                    }
                                }
                            }

                            IconButton {
                                id: menuButton

                                icons.source: Images.threeDots
                                icons.colorEnabled: status === BackupSettingsModel.FAIL ? ColorTheme.textError : ColorTheme.buttonPrimary
                                sizes {
                                    iconWidth: folderSearchIconSize
                                    spacing: 0
                                    textLineHeight: folderSearchIconSize
                                }
                                visible: backupItem.backupItemContainsMouse
                                Layout.alignment: Qt.AlignVCenter
                                onClicked: {
                                    menu.popup(backupItem.width - menu.width, backupItem.height)
                                }
                            }
                        }
                    }

                    ContextMenu {
                        id: menu

                        onFocusChanged: {
                            if (menu.activeFocus === false) {
                                menu.close();
                            }
                        }

                        ContextMenuItem {
                            text: root.getShowInFolderText()
                            icon.source: Images.folder_small_thin_outline
                            onTriggered: {
                                backupSettings.exploreLocalBackup(folder);
                            }
                        }

                        ContextMenuItem {
                            text: SettingsStrings.menuActionsOpenInMega
                            icon.source: Images.mega_small_thin_outline
                            onTriggered: {
                                backupSettings.openInMega(index);
                            }
                        }

                        MenuSeparator {
                        }

                        ContextMenuItem {
                            visible: status === BackupSettingsModel.BACKED_UP
                            height: visible ? implicitHeight : 0
                            text: SettingsStrings.menuActionsPause
                            icon.source: Images.pause_small_thin_outline
                            onTriggered: {
                                backupSettings.pauseBackup(index);
                            }
                        }

                        ContextMenuItem {
                            visible: status === BackupSettingsModel.SUSPENDED
                            height: visible ? implicitHeight : 0
                            text: SettingsStrings.menuActionsResume
                            icon.source: Images.play_small_thin_outline
                            onTriggered: {
                                backupSettings.resumeBackup(index);
                            }
                        }

                        MenuSeparator {
                            visible: status === BackupSettingsModel.BACKED_UP || status === BackupSettingsModel.SUSPENDED
                            height: visible ? implicitHeight : 0
                        }

                        ContextMenuItem {
                            text: SettingsStrings.menuActionsManageExclusions
                            icon.source: Images.slash_circle_small_thin_outline
                            onTriggered: {
                                backupSettings.openExclusionsDialog(index);
                            }
                        }

                        MenuSeparator {
                        }

                        ContextMenuItem {
                            visible: status !== BackupSettingsModel.FAIL && status !== BackupSettingsModel.SUSPENDED
                            height: visible ? implicitHeight : 0
                            text: SettingsStrings.menuActionsRescan
                            icon.source: Images.search_large_small_thin_outline
                            onTriggered: {
                                backupSettings.rescan(index);
                            }
                        }

                        ContextMenuItem {
                            visible: status !== BackupSettingsModel.FAIL && status !== BackupSettingsModel.SUSPENDED
                            height: visible ? implicitHeight : 0
                            text: SettingsStrings.menuActionsRebootBackup
                            icon.source: Images.power_small_thin_outline
                            onTriggered: {
                                backupSettings.reboot(index);
                            }
                        }

                        MenuSeparator {
                            visible: status !== BackupSettingsModel.FAIL && status !== BackupSettingsModel.SUSPENDED
                            height: visible ? implicitHeight : 0
                        }

                        ContextMenuItem {
                            text: SettingsStrings.menuActionsStopBackup
                            icon.source: Images.database_x_medium_thin_outline
                            onTriggered: {
                                backupSettings.remove(index);
                            }
                        }
                    }
                }

                Rectangle {
                    id: errorItem

                    visible: status === BackupSettingsModel.FAIL
                    Layout.bottomMargin: root.errorBorders
                    Layout.rightMargin: Layout.bottomMargin
                    Layout.leftMargin: Layout.bottomMargin
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true
                    Layout.preferredHeight: backupError.implicitHeight
                    color: ColorTheme.notificationError
                    radius: errorBackupItemRadius

                    BackupError {
                        id: backupError

                        errorId: error_id
                        errorMessage: error
                        backupLocalFolder: folder
                    }
                }
            }
        }
    }
}
