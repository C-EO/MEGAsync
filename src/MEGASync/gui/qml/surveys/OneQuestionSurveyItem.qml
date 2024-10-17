import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

import common 1.0

import components.texts 1.0
import components.textFields 1.0
import components.buttons 1.0

//Window {
Rectangle {
    id: root

    color: "#88555555"
    //width: content.width
    //height: content.height

    Rectangle {
        id: content

        readonly property string inputDataView: "inputDataView"
        readonly property string finalView: "finalView"
        readonly property int contentBorderWidth: 1
        readonly property real contentMargin: 24
        readonly property real contentRadius: 8
        readonly property real contentWidth: 250
        readonly property real contentSpacing: 20
        readonly property real titleLineHeight: 24
        readonly property real descriptionLineHeight: 20

        anchors.centerIn: parent
        width: stackViewItem.width + 2 * content.contentMargin
        height: stackViewItem.height + 2 * content.contentMargin
        radius: content.contentRadius
        border {
            color: ColorTheme.borderStrong
            width: content.contentBorderWidth
        }
        color: ColorTheme.pageBackground

        state: content.inputDataView
        states: [
            State {
                name: content.inputDataView
                StateChangeScript {
                    script: stackViewItem.replace(inputDataViewComponent);
                }
            },
            State {
                name: content.finalView
                StateChangeScript {
                    script: stackViewItem.replace(finalViewComponent);
                }
            }
        ]

        StackView {
            id: stackViewItem

            anchors {
                centerIn: parent
                margins: content.contentMargin
            }
            width: content.contentWidth
            height: currentItem.implicitHeight

            Component {
                id: inputDataViewComponent

                Column {
                    id: inputDataViewColumn

                    spacing: content.contentSpacing

                    Text {
                        id: titleItem

                        width: parent.width
                        font {
                            pixelSize: Text.Size.MEDIUM_LARGE
                            weight: Font.Bold
                        }
                        lineHeight: content.titleLineHeight
                        lineHeightMode: Text.FixedHeight
                        horizontalAlignment: Text.AlignHCenter
                        text: "How would you rate your experience uploading to MEGA?"
                    }

                    RateScaleItem {
                        width: parent.width
                    }

                    TextField {
                        id: commentItem

                        anchors {
                            left: parent.left
                            right: parent.right
                            margins: Constants.focusAdjustment
                        }
                        placeholderText: OneQuestionSurveyStrings.tellUsMore
                    }

                    Column {
                        id: buttonsItem

                        anchors {
                            left: parent.left
                            right: parent.right
                            margins: Constants.focusAdjustment
                        }
                        spacing: 0

                        PrimaryButton {
                            anchors {
                                left: parent.left
                                right: parent.right
                            }
                            sizes.fillWidth: true
                            text: OneQuestionSurveyStrings.submit
                            onClicked: {
                                content.state = content.finalView;
                            }
                        }

                        TextButton {
                            anchors {
                                left: parent.left
                                right: parent.right
                            }
                            sizes.fillWidth: true
                            text: Strings.notNow
                            onClicked: {
                                console.log("Not now");
                            }
                        }
                    }

                }

            } // Component: inputDataViewComponent

            Component {
                id: finalViewComponent

                Column {
                    spacing: content.contentSpacing
                    anchors.fill: parent

                    Text {
                        width: parent.width
                        font {
                            pixelSize: Text.Size.MEDIUM_LARGE
                            bold: true
                        }
                        lineHeight: content.titleLineHeight
                        lineHeightMode: Text.FixedHeight
                        horizontalAlignment: Text.AlignHCenter
                        text: OneQuestionSurveyStrings.title
                    }

                    Text {
                        width: parent.width
                        font.pixelSize: Text.Size.MEDIUM
                        lineHeight: content.descriptionLineHeight
                        lineHeightMode: Text.FixedHeight
                        horizontalAlignment: Text.AlignHCenter
                        text: OneQuestionSurveyStrings.description
                    }

                    PrimaryButton {
                        anchors {
                            left: parent.left
                            right: parent.right
                            margins: Constants.focusAdjustment
                        }
                        sizes.fillWidth: true
                        text: OneQuestionSurveyStrings.okGotIt
                        onClicked: {
                            console.log("OK, got it")
                        }
                    }
                }

            } // Component: finalViewComponent

        } // StackView: stackViewItem

    } // Rectangle: content

}
