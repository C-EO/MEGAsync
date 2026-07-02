import QtQuick 2.15

import common 1.0

import components.views 1.0
import components.textFields 1.0

import ChangePasswordComponents 1.0

ChangePasswordDialog {
    id: window

    readonly property int twoFAPageHeigh: 500
    readonly property int passwordChangePageHeigh: 344
    readonly property int passwordChangePageWidth: 496

    title: ChangePasswordStrings.title
    visible: false
    modality: Qt.WindowModal
    color: ColorTheme.pageBackground
    width: passwordChangePageWidth
    height: passwordChangePageHeigh
    maximumHeight: height
    maximumWidth: width
    minimumHeight: height
    minimumWidth: width

    Rectangle {
        id: changePasswordContentItem

        anchors.fill: parent
        color: ColorTheme.pageBackground

        readonly property string change_password: "change_password"
        readonly property string two_fa: "two_fa"

        states: [
            State {
                name: changePasswordContentItem.change_password
                StateChangeScript {
                    script: stackView.replace(changePasswordPage);
                }
            },
            State {
                name: changePasswordContentItem.two_fa
                StateChangeScript {
                    script: {
                        // Resize the window and swap to the 2FA page while it is
                        // hidden, then reveal it (re-centered on its parent) only once
                        // the new size has settled. readyToBeShow() drops the opacity
                        // to 0, so the user never sees the dialog grow with an empty
                        // band underneath (the "blue flash") nor the small->grow jump.
                        // Its hide()/show() also re-applies the geometry constraints,
                        // so the new height sticks without needing a move event.
                        // maximumHeight is raised before height so setHeight is not
                        // clamped by the stale passwordChangePageHeigh maximum.
                        window.maximumHeight = twoFAPageHeigh;
                        window.height = twoFAPageHeigh;
                        window.minimumHeight = twoFAPageHeigh;
                        stackView.replace(twoFAPage);
                        window.readyToBeShow();
                    }
                }
            }
        ]

        StackViewBase {
            id: stackView

            anchors {
                fill: parent
                margins: Constants.bigWindowMargin
            }

            initialItem: changePasswordPage

            Component {
                id: changePasswordPage

                ChangePasswordPage {
                    id: changePasswordItem
                }
            }

            Component {
                id: twoFAPage

                TwoFAPage {
                    id: twoFAItem
                }
            }
        }

        Connections{
            id: changePassConn

            target: changePasswordComponentAccess

            function onShow2FA() {
                changePasswordContentItem.state = changePasswordContentItem.two_fa;
            }

            function onPasswordChangeSucceed() {
                window.close();
            }
        }
    }
}
