import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: sniperWindow
    flags: Qt.Window | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.MaximizeUsingFullscreenGeometryHint
    visibility: Window.FullScreen
    color: "transparent"

    property real startX: 0
    property real startY: 0
    property real currentX: 0
    property real currentY: 0
    property bool isSelecting: false
    property bool hasSelection: false

    property real selX: Math.min(startX, currentX)
    property real selY: Math.min(startY, currentY)
    property real selW: Math.abs(currentX - startX)
    property real selH: Math.abs(currentY - startY)

    // Keyboard Shortcuts
    Item {
        id: keyboardController
        focus: true
        Component.onCompleted: forceActiveFocus()
        Keys.onEscapePressed: {
            captureEngine.cancelCapture()
        }
        Keys.onReturnPressed: {
            confirmCapture("")
        }
        Keys.onEnterPressed: {
            confirmCapture("")
        }
        Keys.onPressed: (event) => {
            if (!sniperWindow.hasSelection)
                return
            const resize = (event.modifiers & Qt.ShiftModifier) !== 0
            if (event.key === Qt.Key_Left) {
                sniperWindow.adjustSelection(-1, 0, resize)
                event.accepted = true
            } else if (event.key === Qt.Key_Right) {
                sniperWindow.adjustSelection(1, 0, resize)
                event.accepted = true
            } else if (event.key === Qt.Key_Up) {
                sniperWindow.adjustSelection(0, -1, resize)
                event.accepted = true
            } else if (event.key === Qt.Key_Down) {
                sniperWindow.adjustSelection(0, 1, resize)
                event.accepted = true
            }
        }
    }

    function adjustSelection(dx, dy, resize) {
        if (resize) {
            currentX = Math.max(0, Math.min(width, currentX + dx))
            currentY = Math.max(0, Math.min(height, currentY + dy))
            return
        }

        const nextX = Math.max(0, Math.min(width - selW, selX + dx))
        const nextY = Math.max(0, Math.min(height - selH, selY + dy))
        startX = nextX
        startY = nextY
        currentX = nextX + selW
        currentY = nextY + selH
    }

    function confirmCapture(action) {
        if (hasSelection && selW > 2 && selH > 2) {
            captureEngine.processRegionSelected(Math.round(selX), Math.round(selY), Math.round(selW), Math.round(selH), action || "")
        } else {
            captureEngine.cancelCapture()
        }
    }

    // Frozen Desktop Image
    Image {
        id: frozenImage
        anchors.fill: parent
        source: "file://" + captureEngine.frozenFramePath
        fillMode: Image.Stretch
        cache: false
    }

    // Dimmed Dark Overlay outside Selection
    Item {
        anchors.fill: parent
        visible: sniperWindow.hasSelection

        // Top Dim
        Rectangle {
            x: 0; y: 0
            width: parent.width
            height: sniperWindow.selY
            color: "#80000000"
        }
        // Bottom Dim
        Rectangle {
            x: 0; y: sniperWindow.selY + sniperWindow.selH
            width: parent.width
            height: parent.height - (sniperWindow.selY + sniperWindow.selH)
            color: "#80000000"
        }
        // Left Dim
        Rectangle {
            x: 0; y: sniperWindow.selY
            width: sniperWindow.selX
            height: sniperWindow.selH
            color: "#80000000"
        }
        // Right Dim
        Rectangle {
            x: sniperWindow.selX + sniperWindow.selW; y: sniperWindow.selY
            width: parent.width - (sniperWindow.selX + sniperWindow.selW)
            height: sniperWindow.selH
            color: "#80000000"
        }
    }

    // Semi-transparent overlay when not selected
    Rectangle {
        anchors.fill: parent
        color: "#30000000"
        visible: !sniperWindow.hasSelection
    }

    // Crosshair Guidelines
    Item {
        anchors.fill: parent
        visible: !sniperWindow.hasSelection

        // Horizontal Guide
        Rectangle {
            x: 0; y: mouseArea.mouseY
            width: parent.width; height: 1
            color: "#603B82F6"
        }
        // Vertical Guide
        Rectangle {
            x: mouseArea.mouseX; y: 0
            width: 1; height: parent.height
            color: "#603B82F6"
        }
    }

    // Selection Box
    Rectangle {
        id: selectionBox
        visible: sniperWindow.hasSelection
        x: sniperWindow.selX
        y: sniperWindow.selY
        width: sniperWindow.selW
        height: sniperWindow.selH
        color: "transparent"
        border.color: "#3B82F6"
        border.width: 2

        // Dimension Badge
        Rectangle {
            x: 0
            y: parent.y > 30 ? -28 : 4
            height: 24
            width: dimText.implicitWidth + 14
            color: "#1E293B"
            border.color: "#3B82F6"
            border.width: 1
            radius: 4

            Text {
                id: dimText
                anchors.centerIn: parent
                text: "%1 × %2 px".arg(Math.round(sniperWindow.selW)).arg(Math.round(sniperWindow.selH))
                color: "#F8FAFC"
                font.pixelSize: 11
                font.bold: true
            }
        }

        // Mini Action Toolbar below selection
        Rectangle {
            id: miniToolbar
            visible: !sniperWindow.isSelecting && sniperWindow.hasSelection && sniperWindow.selW > 60
            x: Math.min(parent.width - width, Math.max(0, parent.width - width))
            y: parent.height + 8 < (sniperWindow.height - sniperWindow.selY - 45) ? parent.height + 8 : parent.height - 45
            height: 38
            width: toolRow.implicitWidth + 16
            color: "#1E293B"
            border.color: "#334155"
            border.width: 1
            radius: 6

            RowLayout {
                id: toolRow
                anchors.centerIn: parent
                spacing: 6

                Button {
                    text: qsTr("📋 Kopyala")
                    onClicked: sniperWindow.confirmCapture("copy")
                    contentItem: Text {
                        text: parent.text
                        color: "#FFFFFF"
                        font.pixelSize: 11
                        font.bold: true
                    }
                    background: Rectangle {
                        implicitWidth: 80
                        implicitHeight: 28
                        color: parent.hovered ? "#2563EB" : "#1D4ED8"
                        radius: 4
                    }
                }

                Button {
                    text: qsTr("💾 Kaydet")
                    onClicked: sniperWindow.confirmCapture("save")
                    contentItem: Text {
                        text: parent.text
                        color: "#E2E8F0"
                        font.pixelSize: 11
                    }
                    background: Rectangle {
                        implicitWidth: 70
                        implicitHeight: 28
                        color: parent.hovered ? "#334155" : "#0F172A"
                        border.color: "#334155"
                        radius: 4
                    }
                }

                Button {
                    text: qsTr("✕")
                    onClicked: captureEngine.cancelCapture()
                    contentItem: Text {
                        text: "✕"
                        color: "#EF4444"
                        font.pixelSize: 12
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                    }
                    background: Rectangle {
                        implicitWidth: 30
                        implicitHeight: 28
                        color: parent.hovered ? "#450A0A" : "#0F172A"
                        border.color: "#7F1D1D"
                        radius: 4
                    }
                }
            }
        }
    }

    // Magnifier Loupe (Floating Pixel Zoom)
    Rectangle {
        id: magnifier
        visible: settingsManager.magnifierEnabled && !sniperWindow.hasSelection
        width: 140
        height: 140
        radius: 70
        color: "#1E293B"
        border.color: "#3B82F6"
        border.width: 3
        clip: true

        // Follow mouse with boundary collision prevention
        x: (mouseArea.mouseX + 20 + width > sniperWindow.width) ? mouseArea.mouseX - width - 20 : mouseArea.mouseX + 20
        y: (mouseArea.mouseY + 20 + height > sniperWindow.height) ? mouseArea.mouseY - height - 20 : mouseArea.mouseY + 20

        Item {
            anchors.fill: parent
            clip: true

            Image {
                id: zoomedImage
                source: frozenImage.source
                width: frozenImage.width * settingsManager.magnifierZoom
                height: frozenImage.height * settingsManager.magnifierZoom
                x: -(mouseArea.mouseX * settingsManager.magnifierZoom - parent.width / 2)
                y: -(mouseArea.mouseY * settingsManager.magnifierZoom - parent.height / 2)
            }

            // Loupe Central Crosshair
            Rectangle {
                anchors.centerIn: parent
                width: parent.width; height: 1
                color: "#80EF4444"
            }
            Rectangle {
                anchors.centerIn: parent
                width: 1; height: parent.height
                color: "#80EF4444"
            }

            // Loupe Coordinate & Info Badge
            Rectangle {
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottomMargin: 8
                height: 20
                width: parent.width - 24
                radius: 4
                color: "#CC0F172A"

                Text {
                    anchors.centerIn: parent
                    text: "%1, %2".arg(Math.round(mouseArea.mouseX)).arg(Math.round(mouseArea.mouseY))
                    color: "#F8FAFC"
                    font.pixelSize: 10
                    font.bold: true
                }
            }
        }
    }

    // Global Mouse Interaction Area
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.CrossCursor

        onPressed: (mouse) => {
            keyboardController.forceActiveFocus()
            sniperWindow.startX = mouse.x
            sniperWindow.startY = mouse.y
            sniperWindow.currentX = mouse.x
            sniperWindow.currentY = mouse.y
            sniperWindow.isSelecting = true
            sniperWindow.hasSelection = true
        }

        onPositionChanged: (mouse) => {
            if (sniperWindow.isSelecting) {
                sniperWindow.currentX = mouse.x
                sniperWindow.currentY = mouse.y
            }
        }

        onReleased: (mouse) => {
            sniperWindow.isSelecting = false
            sniperWindow.currentX = mouse.x
            sniperWindow.currentY = mouse.y

            if (sniperWindow.selW < 4 && sniperWindow.selH < 4) {
                sniperWindow.hasSelection = false
            }
        }

        onDoubleClicked: {
            sniperWindow.confirmCapture("")
        }
    }
}
