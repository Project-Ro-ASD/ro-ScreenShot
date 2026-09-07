import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: busyRoot
    RoMotion { id: motion }

    property bool isBusy: false
    property string text: qsTr("İşleniyor...")
    property var colors: null

    anchors.fill: parent
    visible: opacity > 0.0
    opacity: isBusy ? 1.0 : 0.0

    Behavior on opacity {
        NumberAnimation { duration: motion.standard; easing.type: Easing.OutCubic }
    }

    // Semi-transparent background
    Rectangle {
        anchors.fill: parent
        color: "#000000"
        opacity: 0.46
    }

    Rectangle {
        anchors.centerIn: parent
        width: Math.max(160, layout.implicitWidth + 32)
        height: 60
        radius: 14
        color: (busyRoot.colors && busyRoot.colors.cardStrong) ? busyRoot.colors.cardStrong : "#342D4A"
        border.color: (busyRoot.colors && busyRoot.colors.border) ? busyRoot.colors.border : "#4D436B"
        border.width: 1

        RowLayout {
            id: layout
            anchors.centerIn: parent
            spacing: 12

            BusyIndicator {
                running: busyRoot.isBusy
                implicitWidth: 28
                implicitHeight: 28
            }

            Text {
                text: busyRoot.text
                color: (busyRoot.colors && busyRoot.colors.text) ? busyRoot.colors.text : "#F8FAFC"
                font.pixelSize: 13
                font.bold: true
            }
        }
    }
}
