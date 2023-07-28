import QtQuick
import QtQuick.Controls

Item {
    width: parent.width; height: parent.height
    clip: true

    property int fontBig: 20
    property int fontMedium: 13
    property int fontSmall: 11
    property int fontExtraSmall: 8

    property int buttonWidth: 103
    property int buttonHeight: 25

    property int leftMargin: 48
    property int rightMargin: 16

    property string backgroundColour: virtualstudio.darkMode ? "#272525" : "#FAFBFB"
    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
    property string buttonColour: virtualstudio.darkMode ? "#494646" : "#EAECEC"
    property string buttonHoverColour: virtualstudio.darkMode ? "#5B5858" : "#D3D4D4"
    property string buttonPressedColour: virtualstudio.darkMode ? "#524F4F" : "#DEE0E0"
    property string buttonStroke: virtualstudio.darkMode ? "#80827D7D" : "#34979797"
    property string buttonHoverStroke: virtualstudio.darkMode ? "#7B7777" : "#BABCBC"
    property string buttonPressedStroke: virtualstudio.darkMode ? "#827D7D" : "#BABCBC"
    property string saveButtonBackgroundColour: "#F2F3F3"
    property string saveButtonPressedColour: "#E7E8E8"
    property string saveButtonStroke: "#EAEBEB"
    property string saveButtonPressedStroke: "#B0B5B5"
    property string recommendationText: "#DB0A0A"
    property string saveButtonText: "#DB0A0A"
    property string checkboxStroke: "#0062cc"
    property string checkboxPressedStroke: "#007AFF"
    property string disabledButtonText: "#D3D4D4"
    property string linkText: virtualstudio.darkMode ? "#8B8D8D" : "#272525"

    property bool currShowRecommendations: virtualstudio.showWarnings
    property string recommendationScreen: virtualstudio.showWarnings ? "ethernet" : ( permissions.micPermission == "unknown" ? "microphone" : "acknowledged")
    property bool onWindows: Qt.platform.os === "windows"


    Item {
        id: ethernetRecommendationItem
        width: parent.width; height: parent.height
        visible: recommendationScreen == "ethernet"

        AppIcon {
            id: ethernetRecommendationLogo
            y: 120
            anchors.horizontalCenter: parent.horizontalCenter
            width: 179
            height: 128
            icon.source: "ethernet.svg"
        }

        Text {
            id: ethernetRecommendationHeader1
            text: "Connect via Wired Ethernet"
            font { family: "Poppins"; weight: Font.Bold; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: ethernetRecommendationLogo.bottom
            anchors.topMargin: 32 * virtualstudio.uiScale
        }

        Text {
            id: ethernetRecommendationSubheader1
            text: "JackTrip works best when you connect directly to your home router via a wired ethernet cable. WiFi works OK for some people, but you will experience higher latency and audio glitches."
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            width: 560
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: ethernetRecommendationHeader1.bottom
            anchors.topMargin: 32 * virtualstudio.uiScale
        }

        Button {
            id: okButtonEthernet
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: okButtonEthernet.down ? saveButtonPressedColour : saveButtonBackgroundColour
                border.width: 1
                border.color: okButtonEthernet.down || okButtonEthernet.hovered ? saveButtonPressedStroke : saveButtonStroke
                layer.enabled: okButtonEthernet.hovered && !okButtonEthernet.down
            }
            onClicked: { recommendationScreen = "fiber" }
            anchors.right: parent.right
            anchors.rightMargin: 16 * virtualstudio.uiScale
            anchors.bottomMargin: 16 * virtualstudio.uiScale
            anchors.bottom: parent.bottom
            width: 150 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
            Text {
                text: "Continue"
                font.family: "Poppins"
                font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
                font.weight: Font.Bold
                color: saveButtonText
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    Item {
        id: fiberRecommendationItem
        width: parent.width; height: parent.height
        visible: recommendationScreen == "fiber"

        AppIcon {
            id: fiberRecommendationLogo
            y: 120
            anchors.horizontalCenter: parent.horizontalCenter
            width: 179
            height: 128
            icon.source: "ethernet.svg"
        }

        Text {
            id: fiberRecommendationHeader
            text: "Fiber Internet Recommended"
            font { family: "Poppins"; weight: Font.Bold; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: fiberRecommendationLogo.bottom
            anchors.topMargin: 32 * virtualstudio.uiScale
        }

        Text {
            id: fiberRecommendationSubheader
            text: "Additionally, a Fiber Internet connection from your Internet Service Provider (ISP) will give you the best experience while using JackTrip. You can still use JackTrip with Cable and DSL, but these types of Internet connections introduce significantly higher latency."
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            width: 560
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: fiberRecommendationHeader.bottom
            anchors.topMargin: 32 * virtualstudio.uiScale
        }


        Button {
            id: okButtonFiber
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: okButtonFiber.down ? saveButtonPressedColour : saveButtonBackgroundColour
                border.width: 1
                border.color: okButtonFiber.down || okButtonFiber.hovered ? saveButtonPressedStroke : saveButtonStroke
                layer.enabled: okButtonFiber.hovered && !okButtonFiber.down
            }
            onClicked: { recommendationScreen = "headphones" }
            anchors.right: parent.right
            anchors.rightMargin: 16 * virtualstudio.uiScale
            anchors.bottomMargin: 16 * virtualstudio.uiScale
            anchors.bottom: parent.bottom
            width: 150 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
            Text {
                text: "Continue"
                font.family: "Poppins"
                font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
                font.weight: Font.Bold
                color: saveButtonText
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    Item {
        id: headphoneRecommendationItem
        width: parent.width; height: parent.height
        visible: recommendationScreen == "headphones"

        AppIcon {
            id: headphoneWarningLogo
            y: 120
            anchors.horizontalCenter: parent.horizontalCenter
            width: 118
            height: 128
            icon.source: "headphones.svg"
        }

        Text {
            id: headphoneRecommendationHeader1
            text: "Use Wired Headphones"
            font { family: "Poppins"; weight: Font.Bold; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: headphoneWarningLogo.bottom
            anchors.topMargin: 32 * virtualstudio.uiScale
        }

        Text {
            id: headphoneRecommendationSubheader1
            text: "JackTrip requires the use of wired headphones. Using speakers can cause loud feedback loops, and wireless / bluetooth headphones add way too much latency."
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            width: 560
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: headphoneRecommendationHeader1.bottom
            anchors.topMargin: 32 * virtualstudio.uiScale
        }

        Button {
            id: okButtonHeadphones
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: okButtonHeadphones.down ? saveButtonPressedColour : saveButtonBackgroundColour
                border.width: 1
                border.color: okButtonHeadphones.down || okButtonHeadphones.hovered ? saveButtonPressedStroke : saveButtonStroke
                layer.enabled: okButtonHeadphones.hovered && !okButtonHeadphones.down
            }
            onClicked: {
                recommendationScreen = "audiointerface";
            }
            anchors.right: parent.right
            anchors.rightMargin: 16 * virtualstudio.uiScale
            anchors.bottomMargin: 16 * virtualstudio.uiScale
            anchors.bottom: parent.bottom
            width: 150 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
            Text {
                text: "Continue"
                font.family: "Poppins"
                font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
                font.weight: Font.Bold
                color: saveButtonText
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    Item {
        id: audioInterfaceRecommendationItem
        width: parent.width; height: parent.height
        visible: recommendationScreen == "audiointerface"

        AppIcon {
            id: audioInterfaceRecommendationLogo
            y: 120
            anchors.horizontalCenter: parent.horizontalCenter
            width: 118
            height: 128
            icon.source: "headphones.svg"
        }

        Text {
            id: audioInterfaceRecommendationHeaderNonWindows
            visible: !onWindows
            text: "Use an External Audio Device"
            font { family: "Poppins"; weight: Font.Bold; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: audioInterfaceRecommendationLogo.bottom
            anchors.topMargin: 32 * virtualstudio.uiScale
        }

        Text {
            id: audioInterfaceRecommendationSubheaderNonWindows
            visible: !onWindows
            text: "Your audio device controls the quality of sound, and can also have a big impact on latency."
                + " We recommend using an external USB or Thunderbolt audio interface."
                + "<br/><br/>"
                + "You can still use the audio device that is built into your computer, but you will get the best results when using an external audio device."
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            width: 560
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: audioInterfaceRecommendationHeaderNonWindows.bottom
            anchors.topMargin: 32 * virtualstudio.uiScale
        }

        Text {
            id: audioInterfaceRecommendationHeaderWindows
            visible: onWindows
            text: "Use an External Audio Device"
            font { family: "Poppins"; weight: Font.Bold; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: audioInterfaceRecommendationLogo.bottom
            anchors.topMargin: 32 * virtualstudio.uiScale
        }

        Text {
            id: audioInterfaceRecommendationSubheaderWindows
            visible: onWindows
            text: "Your audio device controls the quality of sound, and can also have a big impact on latency. We recommend using an external USB or Thunderbolt audio interface."
                + "<br/><br/>"
                + "Additionally, low latency on Windows requires the use of ASIO drivers. Beware that the use of ASIO drivers that are not made specifically for your device can cause crashes."
                + "<br/><br/>"
                + "You can still use JackTrip with other audio devices, but you will get the best results when using an external audio device with ASIO drivers."
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            width: 560
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: audioInterfaceRecommendationHeaderWindows.bottom
            anchors.topMargin: 32 * virtualstudio.uiScale
        }

        Button {
            id: okButtonAudioInterface
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: okButtonAudioInterface.down ? saveButtonPressedColour : saveButtonBackgroundColour
                border.width: 1
                border.color: okButtonAudioInterface.down || okButtonAudioInterface.hovered ? saveButtonPressedStroke : saveButtonStroke
                layer.enabled: okButtonAudioInterface.hovered && !okButtonAudioInterface.down
            }
            onClicked: {
                recommendationScreen = "acknowledged";
            }
            anchors.right: parent.right
            anchors.rightMargin: 16 * virtualstudio.uiScale
            anchors.bottomMargin: 16 * virtualstudio.uiScale
            anchors.bottom: parent.bottom
            width: 150 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
            Text {
                text: "Continue"
                font.family: "Poppins"
                font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
                font.weight: Font.Bold
                color: saveButtonText
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    Item {
        id: acknowledgedRecommendationItem
        width: parent.width; height: parent.height
        visible: recommendationScreen == "acknowledged"

        Text {
            id: acknowledgedHeader
            text: "Repeat these recommendations?"
            font { family: "Poppins"; weight: Font.Bold; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 96 * virtualstudio.uiScale
        }

        Text {
            id: acknowledgedSubheader
            text: "Would you like to repeat these recommendations the next time JackTrip starts up?"
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            width: 560
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: acknowledgedHeader.bottom
            anchors.topMargin: 64 * virtualstudio.uiScale
        }

        Item {
            id: acknowledgedButtonsContainer
            width: 320

            anchors.top: acknowledgedSubheader.bottom
            anchors.topMargin: 96 * virtualstudio.uiScale
            anchors.horizontalCenter: parent.horizontalCenter

            Button {
                id: acknowledgedYesButton
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                background: Rectangle {
                    radius: 6 * virtualstudio.uiScale
                    color: acknowledgedYesButton.down ? saveButtonPressedColour : saveButtonBackgroundColour
                    border.width: 1
                    border.color: acknowledgedYesButton.down || acknowledgedYesButton.hovered ? saveButtonPressedStroke : saveButtonStroke
                    layer.enabled: acknowledgedYesButton.hovered && !acknowledgedYesButton.down
                }
                onClicked: {
                    virtualstudio.showWarnings = true;
                    if (permissions.micPermission === "granted") {
                        virtualstudio.windowState = "browse";
                        virtualstudio.applySettings();
                    } else {
                        virtualstudio.windowState = "permissions";
                        virtualstudio.applySettings();
                    }
                }
                width: 150 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
                Text {
                    text: "Yes"
                    font.family: "Poppins"
                    font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
                    font.weight: Font.Bold
                    color: saveButtonText
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
            }


            Button {
                id: acknowledgedNoButton
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                background: Rectangle {
                    radius: 6 * virtualstudio.uiScale
                    color: acknowledgedNoButton.down ? saveButtonPressedColour : saveButtonBackgroundColour
                    border.width: 1
                    border.color: acknowledgedNoButton.down || acknowledgedNoButton.hovered ? saveButtonPressedStroke : saveButtonStroke
                    layer.enabled: acknowledgedNoButton.hovered && !acknowledgedNoButton.down
                }
                onClicked: {
                    virtualstudio.showWarnings = false;
                    if (permissions.micPermission === "granted") {
                        virtualstudio.windowState = "browse";
                        virtualstudio.applySettings();
                    } else {
                        virtualstudio.windowState = "permissions";
                        virtualstudio.applySettings();
                    }
                }
                width: 150 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
                Text {
                    text: "No"
                    font.family: "Poppins"
                    font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
                    font.weight: Font.Bold
                    color: saveButtonText
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }


        Text {
            id: acknowledgedSettingsInfo
            text: "You can change this setting at any time under <b>Settings > Advanced</b>"
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            width: 560
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: acknowledgedButtonsContainer.bottom
            anchors.topMargin: 96 * virtualstudio.uiScale
        }
    }
}
