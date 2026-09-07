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
    width: 344
    height: 62
    property real dragStartX: 0
    property real dragStartY: 0
    property real windowStartX: 0
    property real windowStartY: 0

    // Default position at bottom center
    x: (Screen.width - width) / 2
    y: Screen.height - height - 48

    Rectangle {
        id: hudCard
        anchors.fill: parent
        radius: 18
        color: "#1B1F2A"
        border.color: "#454D60"
        border.width: 1

        // Drag area for movable HUD
        MouseArea {
            id: dragArea
            anchors.fill: parent
            cursorShape: Qt.SizeAllCursor
            onPressed: function (mouse) {
                hudWindow.dragStartX = mouse.x;
                hudWindow.dragStartY = mouse.y;
                hudWindow.windowStartX = hudWindow.x;
                hudWindow.windowStartY = hudWindow.y;
            }
            onPositionChanged: function (mouse) {
                if (pressed) {
                    hudWindow.x = hudWindow.windowStartX + mouse.x - hudWindow.dragStartX;
                    hudWindow.y = hudWindow.windowStartY + mouse.y - hudWindow.dragStartY;
                }
            }
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            spacing: 12

            // Recording Pulsing Dot
            Rectangle {
                Layout.preferredWidth: 12
                Layout.preferredHeight: 12
                radius: 6
                color: "#D85467"
                SequentialAnimation on opacity {
                    loops: Animation.Infinite
                    running: screenRecorderEngine ? screenRecorderEngine.isRecording && !screenRecorderEngine.isPaused : false
                    NumberAnimation {
                        from: 1.0
                        to: 0.3
                        duration: 600
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        from: 0.3
                        to: 1.0
                        duration: 600
                        easing.type: Easing.InOutQuad
                    }
                }
            }

            // Duration Label
            Text {
                text: screenRecorderEngine ? screenRecorderEngine.formattedDuration : "00:00"
                color: "#F5F6FA"
                font.pixelSize: 15
                font.bold: true
                font.family: "Monospace"
                Layout.alignment: Qt.AlignVCenter
            }

            // Live VU Meter Bar
            Rectangle {
                Layout.preferredWidth: 36
                Layout.preferredHeight: 8
                radius: 4
                color: "#303746"
                Layout.alignment: Qt.AlignVCenter

                Rectangle {
                    height: parent.height
                    radius: 4
                    width: parent.width * (screenRecorderEngine ? screenRecorderEngine.audioLevel : 0.0)
                    color: width > 26 ? "#D85467" : (width > 16 ? "#D79A32" : "#20B58A")
                    Behavior on width {
                        NumberAnimation {
                            duration: 80
                        }
                    }
                }
            }

            Item {
                Layout.fillWidth: true
            }

            // Mic Toggle
            Button {
                flat: true
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
                background: Rectangle {
                    radius: 16
                    color: (screenRecorderEngine && screenRecorderEngine.micEnabled) ? "#5A5FE8" : "#303746"
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
                    color: "#303746"
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
                    color: "#C4475D"
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
