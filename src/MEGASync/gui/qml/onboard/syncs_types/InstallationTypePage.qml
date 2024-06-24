import QtQuick 2.15

InstallationTypePageForm {
    id: root

    signal installationTypeMoveToBack
    signal installationTypeMoveToSync
    signal installationTypeMoveToBackup

    footerButtons {

        rightSecondary.onClicked: {
            root.installationTypeMoveToBack();
        }

        rightPrimary.onClicked: {
            switch(buttonGroup.checkedButton.type) {
                case SyncsType.Types.SYNC:
                    root.installationTypeMoveToSync();
                    break;
                case SyncsType.Types.BACKUP:
                    root.installationTypeMoveToBackup();
                    break;
                default:
                    console.error("Button type does not exist -> "
                                  + buttonGroup.checkedButton.type);
                    break;
            }
        }
    }

    buttonGroup.onCheckStateChanged: {
        if(buttonGroup.checkedButton != null) {
            footerButtons.rightPrimary.enabled = true;
        }
    }

    Connections {
        target: window

        function onInitializePageFocus() {
            syncButton.forceActiveFocus();
        }
    }
}
