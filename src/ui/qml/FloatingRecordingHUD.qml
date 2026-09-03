import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

Window {
    id: hudWindow
    title: "ro-ScreenShot Recording HUD"
    flags: Qt.WindowStaysOnTopHint | Qt.FramelessWindowHint | Qt.Tool
    color: "transparent"
    visible: screenRecorderEngine ? screenRecorderEngine.isRecording : false
    width: 320
    height: 56

    // Default position at bottom center
    x: (Screen.width - width) / 2
    y: Screen.height - height - 48

    Rectangle {
        id: hudCard
        anchors.fill: parent
        radius: 28
        color: "#1E293B"
        border.color: "#334155"
        border.width: 1

        // Drag area for movable HUD
        MouseArea {
            id: dragArea
            anchors.fill: parent
            drag.target: hudWindow
            cursorShape: Qt.SizeAllCursor
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            spacing: 12

            // Recording Pulsing Dot
            Rectangle {
                width: 12
                height: 12
                radius: 6
                color: "#EF4444"
                SequentialAnimation on opacity {
                    loops: Animation.Infinite
                    running: screenRecorderEngine ? screenRecorderEngine.isRecording && !screenRecorderEngine.isPaused : false
                    NumberAnimation { from: 1.0; to: 0.3; duration: 600; easing.type: Easing.InOutQuad }
                    NumberAnimation { from: 0.3; to: 1.0; duration: 600; easing.type: Easing.InOutQuad }
                }
            }

            // Duration Label
            Text {
                text: screenRecorderEngine ? screenRecorderEngine.formattedDuration : "00:00"
                color: "#F8FAFC"
                font.pixelSize: 15
                font.bold: true
                font.family: "Monospace"
                Layout.alignment: Qt.AlignVCenter
            }

            // Live VU Meter Bar
            Rectangle {
                width: 36
                height: 8
                radius: 4
                color: "#334155"
                Layout.alignment: Qt.AlignVCenter

                Rectangle {
                    height: parent.height
                    radius: 4
                    width: parent.width * (screenRecorderEngine ? screenRecorderEngine.audioLevel : 0.0)
                    color: width > 26 ? "#EF4444" : (width > 16 ? "#F59E0B" : "#10B981")
                    Behavior on width { NumberAnimation { duration: 80 } }
                }
            }

            Item { Layout.fillWidth: true }

            // Mic Toggle
            Button {
                flat: true
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
                background: Rectangle {
                    radius: 16
                    color: (screenRecorderEngine && screenRecorderEngine.micEnabled) ? "#3B82F6" : "#334155"
                }
                contentItem: Text {
                    text: (screenRecorderEngine && screenRecorderEngine.micEnabled) ? "🎙" : "🔇"
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    if (screenRecorderEngine) {
                        screenRecorderEngine.micEnabled = !screenRecorderEngine.micEnabled;
                    }
                }
            }

            // Pause / Resume Button
            Button {
                flat: true
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
                background: Rectangle {
                    radius: 16
                    color: "#334155"
                }
                contentItem: Text {
                    text: (screenRecorderEngine && screenRecorderEngine.isPaused) ? "▶" : "⏸"
                    color: "white"
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    if (screenRecorderEngine) {
                        if (screenRecorderEngine.isPaused) {
                            screenRecorderEngine.resumeRecording();
                        } else {
                            screenRecorderEngine.pauseRecording();
                        }
                    }
                }
            }

            // Stop Recording Button
            Button {
                flat: true
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
                background: Rectangle {
                    radius: 16
                    color: "#EF4444"
                }
                contentItem: Text {
                    text: "⏹"
                    color: "white"
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    if (screenRecorderEngine) {
                        screenRecorderEngine.stopRecording();
                    }
                }
            }
        }
    }
}
