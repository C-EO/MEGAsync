//Local
import Onboard 1.0

// C++
import QmlDeviceName 1.0

import Common 1.0

DeviceNamePageForm {
    id: root

    signal moveToSyncType

    footerButtons.rightPrimary.onClicked: {
        var emptyText = deviceNameTextField.text.length === 0;
        if(emptyText)
        {
            deviceNameTextField.hint.styles.textColor = Styles.textError;
        }
        deviceNameTextField.error = emptyText;
        deviceNameTextField.hint.text = emptyText ? OnboardingStrings.errorEmptyDeviceName : "";
        deviceNameTextField.hint.visible = emptyText;

        if(emptyText) {
            return;
        }

        if(!deviceName.setDeviceName(deviceNameTextField.text)) {
            root.moveToSyncType()
        }
    }

    deviceNameTextField.onTextChanged: {
        deviceNameTextField.error = false;
        deviceNameTextField.hint.text = "";
        deviceNameTextField.hint.visible = false;

        if(deviceNameTextField.text.length >= deviceNameTextField.textField.maximumLength)
        {
            deviceNameTextField.hint.styles.textColor = Styles.textSecondary;
            deviceNameTextField.hint.text = OnboardingStrings.errorDeviceNameLimit;
            deviceNameTextField.hint.visible = true;
        }
    }

    QmlDeviceName {
        id: deviceName

        onDeviceNameSet: {
            root.moveToSyncType()
        }
    }
}

