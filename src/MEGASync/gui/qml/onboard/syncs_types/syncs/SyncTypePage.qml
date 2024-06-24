import QtQuick 2.15

import onboard.syncs_types 1.0

SyncTypePageForm {
    id: root

    signal syncTypeMoveToBack;
    signal syncTypeMoveToFullSync;
    signal syncTypeMoveToSelectiveSync;

    footerButtons {

        rightSecondary.onClicked: {
            root.syncTypeMoveToBack()
        }

        rightPrimary.onClicked: {
            switch(buttonGroup.checkedButton.type) {
                case SyncsType.Types.FULL_SYNC:
                    root.syncTypeMoveToFullSync();
                    break;
                case SyncsType.Types.SELECTIVE_SYNC:
                    root.syncTypeMoveToSelectiveSync();
                    break;
                default:
                    console.error("Button type does not exist -> "
                                  + buttonGroup.checkedButton.syncType);
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
            fullSyncButton.forceActiveFocus();
        }
    }
}


