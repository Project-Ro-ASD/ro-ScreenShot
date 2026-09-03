import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

Window {
    id: toastWindow

    width: 320
    height: 100
    flags: Qt.Window | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.SubWindow
    color: "transparent"

    property string imagePath: ""
    property string fileName: ""
    property bool hasFile: imagePath.length > 0

    // Position dynamically in bottom right of active screen taking DPI and available space into account
    x: Screen.virtualX + Screen.desktopAvailableWidth - width - 24
    y: Screen.virtualY + Screen.desktopAvailableHeight - height - 48

    Shortcut {
        sequence: "Escape"
        onActivated: toastWindow.dismiss()
    }

    Timer {
        id: hideTimer
        interval: 4500
        repeat: false
        onTriggered: toastWindow.dismiss()
    }

    function showToast(path, name) {
        imagePath = path || ""
        fileName = name || qsTr("Ekran Görüntüsü")
        hasFile = imagePath.length > 0
        visible = true
        hideTimer.restart()
    }

    function dismiss() {
        hideTimer.stop()
        visible = false
    }

    Rectangle {
        anchors.fill: parent
        color: "#131D31"
        border.color: "#3B82F6"
        border.width: 1.5
        radius: 10

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: hideTimer.stop()
            onExited: hideTimer.restart()
        }

        RowLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 12

            // Thumbnail Image Card
            Rectangle {
                width: 76
                height: 76
                radius: 6
                color: "#0F172A"
                border.color: "#334155"
                border.width: 1
                clip: true

                Image {
                    anchors.fill: parent
                    source: toastWindow.imagePath ? ("file://" + toastWindow.imagePath) : ""
                    fillMode: Image.PreserveAspectFit
                    asynchronous: true
                }

                // Fallback icon if no file or loading
                Text {
                    anchors.centerIn: parent
                    text: "📷"
                    font.pixelSize: 24
                    visible: !toastWindow.imagePath
                }
            }

            // Info & Buttons Column
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    Text {
                        text: qsTr("Ekran Görüntüsü Alındı")
                        color: "#38BDF8"
                        font.pixelSize: 12
                        font.bold: true
                        Layout.fillWidth: true
                    }

                    Button {
                        flat: true
                        onClicked: toastWindow.dismiss()
                        contentItem: Text {
                            text: "✕"
                            color: "#94A3B8"
                            font.pixelSize: 12
                            horizontalAlignment: Text.AlignHCenter
                        }
                        background: Item {
                            implicitWidth: 20
                            implicitHeight: 20
                        }
                        Accessible.name: qsTr("Kapat")
                    }
                }

                Text {
                    text: toastWindow.fileName
                    color: "#F8FAFC"
                    font.pixelSize: 11
                    elide: Text.ElideMiddle
                    Layout.fillWidth: true
                }

                RowLayout {
                    spacing: 6
                    Layout.fillWidth: true

                    // Copy Button
                    Button {
                        text: qsTr("Kopyala")
                        onClicked: {
                            captureEngine.copyImageToClipboard(toastWindow.imagePath)
                            toastWindow.dismiss()
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "#FFFFFF"
                            font.pixelSize: 10
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                        }
                        background: Rectangle {
                            implicitWidth: 64
                            implicitHeight: 24
                            color: parent.hovered ? "#2563EB" : "#1D4ED8"
                            radius: 4
                        }
                        Accessible.name: qsTr("Panoya kopyala")
                    }

                    // Open / Folder buttons if file exists
                    Button {
                        visible: toastWindow.hasFile
                        text: qsTr("Klasör")
                        onClicked: {
                            libraryManager.openInFolder(0)
                            toastWindow.dismiss()
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "#CBD5E1"
                            font.pixelSize: 10
                            horizontalAlignment: Text.AlignHCenter
                        }
                        background: Rectangle {
                            implicitWidth: 54
                            implicitHeight: 24
                            color: parent.hovered ? "#334155" : "#1E293B"
                            border.color: "#334155"
                            radius: 4
                        }
                        Accessible.name: qsTr("Klasörde göster")
                    }
                }
            }
        }
    }
}
