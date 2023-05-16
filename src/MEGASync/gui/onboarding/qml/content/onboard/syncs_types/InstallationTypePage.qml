InstallationTypePageForm {

    footerButtons {

        previousButton.onClicked: {
            syncsFlow.state = computerName;
        }

        nextButton.enabled: false
        nextButton.onClicked: {
            switch(installationTypePage.buttonGroup.checkedButton.type) {
                case SyncsType.Sync:
                    syncsFlow.state = syncs;
                    break;
                case SyncsType.Backup:
                    syncsFlow.state = selectBackup;
                    break;
                case SyncsType.Fuse:
                default:
                    console.error("Button type does not exist -> "
                                  + installationTypePage.buttonGroup.checkedButton.type);
                    break;
            }
        }
    }

    buttonGroup.onCheckStateChanged: {
        if(buttonGroup.checkedButton != null) {
            footerButtons.nextButton.enabled = true;
        }
    }

}
