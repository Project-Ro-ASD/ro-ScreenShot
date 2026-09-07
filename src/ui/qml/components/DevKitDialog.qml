import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RoDialog {
    id: devKitDialog
    title: "Tasarımcı & Geliştirici Araç Kiti"
    modal: true
    width: 680
    height: 520
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    property var paletteData: []
    property string extractedTableText: ""

    header: Rectangle {
        height: 52
        color: devKitDialog.shellColor
        radius: devKitDialog.surfaceRadius

        Text {
            anchors.centerIn: parent
            text: "🛠️ Tasarımcı & Geliştirici Araç Kiti"
            color: devKitDialog.textColor
            font.pixelSize: 16
            font.bold: true
        }
    }

    contentItem: ColumnLayout {
        spacing: 14

        Text {
            text: "🎨 Tailwind CSS Renk Paleti:"
            color: devKitDialog.accentColor
            font.bold: true
            font.pixelSize: 14
        }

        RowLayout {
            spacing: 10
            Repeater {
                model: paletteData
                Rectangle {
                    width: 72
                    height: 64
                    radius: 8
                    color: modelData.hex || "#334155"
                    border.color: "white"
                    border.width: 1

                    Column {
                        anchors.centerIn: parent
                        Text {
                            text: modelData.hex || ""
                            color: "white"
                            font.pixelSize: 10
                            font.bold: true
                            style: Text.Outline
                            styleColor: "black"
                        }
                        Text {
                            text: modelData.tailwindName || ""
                            color: "white"
                            font.pixelSize: 9
                            style: Text.Outline
                            styleColor: "black"
                        }
                    }
                }
            }
        }

        Text {
            text: "📊 Görselden Çıkarılan Tablo (Markdown / CSV / JSON):"
            color: devKitDialog.accentColor
            font.bold: true
            font.pixelSize: 14
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: devKitDialog.shellColor
            radius: 6
            border.color: devKitDialog.borderColor

            ScrollView {
                anchors.fill: parent
                anchors.margins: 8
                clip: true

                TextEdit {
                    text: extractedTableText
                    readOnly: true
                    color: devKitDialog.textColor
                    font.family: "Monospace"
                    font.pixelSize: 12
                    wrapMode: TextEdit.Wrap
                    selectByMouse: true
                }
            }
        }
    }

    footer: DialogButtonBox {
        background: Rectangle { color: devKitDialog.shellColor; radius: devKitDialog.surfaceRadius }
        Button {
            text: "Panoya Kopyala"
            DialogButtonBox.buttonRole: DialogButtonBox.ActionRole
            background: Rectangle { radius: 6; color: "#3B82F6" }
            contentItem: Text { text: "Panoya Kopyala"; color: "white"; horizontalAlignment: Text.AlignHCenter }
            onClicked: {
                if (captureEngine) {
                    captureEngine.copyImageToClipboard(extractedTableText);
                }
            }
        }
        Button {
            text: "Kapat"
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
            background: Rectangle { radius: 8; color: devKitDialog.elevatedColor }
            contentItem: Text { text: "Kapat"; color: "white"; horizontalAlignment: Text.AlignHCenter }
        }
    }
}
