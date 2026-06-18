import QtQuick 2.15

import components.buttons 1.0

/*
  A PrimaryButton used to resolve a sync/backup error. It owns the shared
  "action button" behaviour: it is visible only for a given set of error codes,
  and when clicked it fires actionTriggered(), enters a busy state for
  resetInterval ms (optionally spinning its left icon), then resets.
*/
PrimaryButton {
    id: root

    property int errorId: -1
    // Error codes (SyncErrors.*) for which this button is shown.
    property var triggerErrors: []
    // Always show the button regardless of errorId (e.g. the remove button).
    property bool alwaysVisible: false
    // Allows a button to be disabled for a whole item type (e.g. restore is
    // sync-only); when false the button is never shown nor triggered.
    property bool enabledForType: true
    // Whether the left icon spins while the button is busy.
    property bool spins: false
    property int resetInterval: 5000
    property int spinDuration: 1000

    signal actionTriggered()

    visible: enabledForType && (alwaysVisible || triggerErrors.indexOf(errorId) !== -1)
    checkable: true
    icons.position: Icon.Position.LEFT

    Timer {
        id: busyTimer

        interval: root.resetInterval
        repeat: false

        onRunningChanged: {
            if (running) {
                root.actionTriggered();

                root.checked = true;
                root.buttonCursorShape = Qt.ArrowCursor;
                if (root.spins) {
                    root.leftIconRotation.duration = root.spinDuration;
                    root.leftIconRotation.loops = root.resetInterval / root.spinDuration;
                    root.leftIconRotation.start();
                }
            }
            else {
                root.checked = false;
                root.buttonCursorShape = Qt.PointingHandCursor;
            }
        }
    }

    onClicked: {
        if (!busyTimer.running) {
            busyTimer.start();
        }
    }
}
