import QtQuick
import QtQuick.Window

Window {
    id: presentationWin
    flags: Qt.WindowStaysOnTopHint | Qt.FramelessWindowHint | Qt.WindowTransparentForInput
    color: "transparent"
    visible: false
    x: 0
    y: 0
    width: Screen.width
    height: Screen.height

    property var presentationEngine: null

    // Keystroke Badge
    Rectangle {
        id: keyBadge
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 48
        anchors.horizontalCenter: parent.horizontalCenter
        visible: presentationEngine ? (presentationEngine.currentKeystroke !== "" && presentationEngine.isKeystrokeOverlayActive) : false
        height: 48
        radius: 24
        color: "#0F172ACC"
        border.color: "#38BDF8"
        border.width: 2
        width: keyText.implicitWidth + 32

        Text {
            id: keyText
            anchors.centerIn: parent
            text: presentationEngine ? presentationEngine.currentKeystroke : ""
            color: "#38BDF8"
            font.pixelSize: 18
            font.bold: true
        }

        Behavior on opacity { NumberAnimation { duration: 150 } }
    }
}
