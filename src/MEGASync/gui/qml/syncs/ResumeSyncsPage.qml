import QtQuick 2.15

import common 1.0

ResumeSyncsPageForm {
    id: root

    footerButtons {

        rightSecondary.onClicked: {
            syncsComponentAccess.viewSyncsInSettingsButtonClicked();
            window.accept();
        }

        rightPrimary.onClicked: {
            window.accept();
        }
    }

}
