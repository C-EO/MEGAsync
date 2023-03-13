import QtQuick 2.12
import QtQuick.Controls 2.12

import Onboarding 1.0

LoginPageForm {

    createAccountButton.onClicked: {
        registerStack.replace(registerPage)
    }

    loginButton.onClicked: {
        if(email.length !== 0 && password.length !== 0) {
            Onboarding.onLoginClicked({[Onboarding.OnboardEnum.EMAIL]: email,
                                       [Onboarding.OnboardEnum.PASSWORD]: password})
        }
    }

    forgotPasswordHyperlinkArea.onClicked: {
        Onboarding.onForgotPasswordClicked();
    }

    Component{
        id: registerPage

        RegisterPage {}
    }
}



