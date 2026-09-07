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
        height: 52
        radius: 15
        color: "#E01B1F2A"
        border.color: "#777CFF"
        border.width: 1
        width: keyText.implicitWidth + 38

        Text {
            id: keyText
            anchors.centerIn: parent
            text: presentationEngine ? presentationEngine.currentKeystroke : ""
            color: "#F5F6FA"
            font.pixelSize: 18
            font.bold: true
        }

        Behavior on opacity {
            NumberAnimation {
                duration: 150
            }
        }
    }
}
