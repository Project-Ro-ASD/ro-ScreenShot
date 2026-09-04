import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: toastWindow
    width: 320
    height: 92
    flags: Qt.Window | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.SubWindow
    color: "transparent"

    property string imagePath: ""
    property string fileName: ""
    property bool hasFile: imagePath.length > 0

    x: Screen.virtualX + Screen.desktopAvailableWidth - width - 24
    y: Screen.virtualY + Screen.desktopAvailableHeight - height - 48

    Shortcut { sequence: "Escape"; onActivated: toastWindow.dismiss() }
    Timer { id: hideTimer; interval: 4500; repeat: false; onTriggered: toastWindow.dismiss() }

    function showToast(path, name) {
        imagePath = path || ""
        fileName = name || qsTr("Screenshot")
        hasFile = imagePath.length > 0
        visible = true
        hideTimer.restart()
    }
    function dismiss() { hideTimer.stop(); visible = false }

    Rectangle {
        anchors.fill: parent
        color: "#111827"
        border.color: "#2A3448"
        border.width: 1
        radius: 12

        MouseArea { anchors.fill: parent; hoverEnabled: true; onEntered: hideTimer.stop(); onExited: hideTimer.restart() }

        RowLayout {
            anchors.fill: parent; anchors.margins: 10; spacing: 10
            Rectangle {
                Layout.preferredWidth: 68; Layout.preferredHeight: 68
                radius: 8; color: "#1F2937"; border.color: "#2A3448"; border.width: 1; clip: true
                Image { anchors.fill: parent; source: toastWindow.imagePath ? ("file://" + toastWindow.imagePath) : ""; fillMode: Image.PreserveAspectCrop; asynchronous: true }
            }
            ColumnLayout {
                Layout.fillWidth: true; spacing: 2
                RowLayout {
                    Layout.fillWidth: true; spacing: 6
                    Rectangle { width: 7; height: 7; radius: 3.5; color: "#10B981" }
                    Text { text: qsTr("Screenshot ready"); color: "#F9FAFB"; font.pixelSize: 12; font.bold: true; Layout.fillWidth: true; elide: Text.ElideRight }
                    Text { text: "✕"; color: "#9CA3AF"; font.pixelSize: 12; MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: toastWindow.dismiss() } }
                }
                Text { text: toastWindow.fileName; color: "#9CA3AF"; font.pixelSize: 11; elide: Text.ElideMiddle; Layout.fillWidth: true }
                RowLayout {
                    spacing: 6; Layout.topMargin: 4
                    Button {
                        text: qsTr("Copy")
                        onClicked: { captureEngine.copyImageToClipboard(toastWindow.imagePath); toastWindow.dismiss() }
                        contentItem: Text { text: parent.text; color: "#FFFFFF"; font.pixelSize: 11; font.bold: true; horizontalAlignment: Text.AlignHCenter }
                        background: Rectangle { implicitWidth: 62; implicitHeight: 26; color: parent.hovered ? "#1D4ED8" : "#2563EB"; radius: 6 }
                    }
                    Button {
                        visible: toastWindow.hasFile
                        text: qsTr("Show in folder")
                        onClicked: { libraryManager.openInFolder(0); toastWindow.dismiss() }
                        contentItem: Text { text: parent.text; color: "#D1D5DB"; font.pixelSize: 11; horizontalAlignment: Text.AlignHCenter }
                        background: Rectangle { implicitWidth: 96; implicitHeight: 26; color: parent.hovered ? "#2A3448" : "#1F2937"; radius: 6 }
                    }
                }
            }
        }
    }
}
