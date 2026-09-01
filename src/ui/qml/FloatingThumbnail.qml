import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: toastWindow
    width: 290
    height: 90
    flags: Qt.Window | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.SubWindow
    color: "transparent"

    property string imagePath: ""
    property string fileName: ""

    // Position in bottom right corner of screen
    x: Screen.width - width - 24
    y: Screen.height - height - 48

    Timer {
        id: hideTimer
        interval: 4000
        repeat: false
        onTriggered: toastWindow.visible = false
    }

    function showToast(path, name) {
        imagePath = path
        fileName = name
        visible = true
        hideTimer.restart()
    }

    Rectangle {
        anchors.fill: parent
        color: "#1E293B"
        border.color: "#3B82F6"
        border.width: 1.5
        radius: 10

        RowLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 10

            // Thumbnail
            Rectangle {
                width: 70
                height: 70
                radius: 6
                color: "#0F172A"
                clip: true

                Image {
                    anchors.fill: parent
                    source: toastWindow.imagePath ? "file://" + toastWindow.imagePath : ""
                    fillMode: Image.PreserveAspectFit
                    asynchronous: true
                }
            }

            // Info & Buttons
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Text {
                    text: qsTr("📸 Ekran Görüntüsü Alındı")
                    color: "#38BDF8"
                    font.pixelSize: 12
                    font.bold: true
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

                    Button {
                        text: qsTr("📋 Kopyala")
                        onClicked: {
                            captureEngine.copyImageToClipboard(toastWindow.imagePath)
                            toastWindow.visible = false
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "#FFFFFF"
                            font.pixelSize: 10
                            font.bold: true
                        }
                        background: Rectangle {
                            implicitWidth: 64
                            implicitHeight: 22
                            color: parent.hovered ? "#2563EB" : "#1D4ED8"
                            radius: 4
                        }
                    }

                    Button {
                        text: qsTr("✕")
                        onClicked: toastWindow.visible = false
                        contentItem: Text {
                            text: "✕"
                            color: "#94A3B8"
                            font.pixelSize: 11
                            horizontalAlignment: Text.AlignHCenter
                        }
                        background: Rectangle {
                            implicitWidth: 24
                            implicitHeight: 22
                            color: parent.hovered ? "#334155" : "transparent"
                            radius: 4
                        }
                    }
                }
            }
        }
    }
}
