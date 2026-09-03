import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: diffDialog
    title: "Görsel Karşılaştırma / Diff Modu"
    modal: true
    width: 800
    height: 600
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    property string imagePathA: ""
    property string imagePathB: ""
    property real splitRatio: 0.5
    property var diffStats: ({})

    background: Rectangle {
        color: "#0F172A"
        radius: 12
        border.color: "#334155"
        border.width: 1
    }

    header: Rectangle {
        height: 52
        color: "#1E293B"
        radius: 12

        RowLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 12

            Text {
                text: "🔍 Görsel Karşılaştırma (Diff)"
                color: "#F8FAFC"
                font.pixelSize: 16
                font.bold: true
                Layout.fillWidth: true
            }

            Text {
                text: diffStats.similarityPercent ? "Benzerlik: %" + diffStats.similarityPercent.toFixed(1) : ""
                color: "#38BDF8"
                font.bold: true
                font.pixelSize: 14
            }
        }
    }

    contentItem: Item {
        Rectangle {
            id: canvasArea
            anchors.fill: parent
            anchors.margins: 12
            color: "#020617"
            radius: 8
            clip: true

            // Image B (Background / Right)
            Image {
                id: imgB
                anchors.fill: parent
                source: imagePathB ? "file://" + imagePathB : ""
                fillMode: Image.PreserveAspectFit
            }

            // Image A (Clipped Left side)
            Item {
                id: clipA
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: parent.width * splitRatio
                clip: true

                Image {
                    id: imgA
                    width: canvasArea.width
                    height: canvasArea.height
                    source: imagePathA ? "file://" + imagePathA : ""
                    fillMode: Image.PreserveAspectFit
                }
            }

            // Splitter line & handle
            Rectangle {
                id: dividerLine
                x: parent.width * splitRatio - 1
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 2
                color: "#38BDF8"

                Rectangle {
                    anchors.centerIn: parent
                    width: 28
                    height: 28
                    radius: 14
                    color: "#38BDF8"
                    border.color: "white"
                    border.width: 2

                    Text {
                        anchors.centerIn: parent
                        text: "↔"
                        color: "white"
                        font.bold: true
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.SplitHCursor
                onPositionChanged: (mouse) => {
                    if (pressed) {
                        splitRatio = Math.max(0.05, Math.min(0.95, mouse.x / width))
                    }
                }
                onPressed: (mouse) => {
                    splitRatio = Math.max(0.05, Math.min(0.95, mouse.x / width))
                }
            }
        }
    }

    footer: DialogButtonBox {
        background: Rectangle { color: "#1E293B"; radius: 12 }
        Button {
            text: "Kapat"
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
            background: Rectangle { radius: 6; color: "#334155" }
            contentItem: Text { text: "Kapat"; color: "white"; horizontalAlignment: Text.AlignHCenter }
        }
    }
}
