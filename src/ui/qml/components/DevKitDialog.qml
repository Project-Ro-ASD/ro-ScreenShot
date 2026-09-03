import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: devKitDialog
    title: "Tasarımcı & Geliştirici Araç Kiti"
    modal: true
    width: 680
    height: 520
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    property var paletteData: []
    property string extractedTableText: ""

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

        Text {
            anchors.centerIn: parent
            text: "🛠️ Tasarımcı & Geliştirici Araç Kiti"
            color: "#F8FAFC"
            font.pixelSize: 16
            font.bold: true
        }
    }

    contentItem: ColumnLayout {
        spacing: 14

        Text {
            text: "🎨 Tailwind CSS Renk Paleti:"
            color: "#38BDF8"
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
            color: "#38BDF8"
            font.bold: true
            font.pixelSize: 14
        }

        TextArea {
            Layout.fillWidth: true
            Layout.fillHeight: true
            text: extractedTableText
            readOnly: true
            color: "#F8FAFC"
            font.family: "Monospace"
            font.pixelSize: 12
            background: Rectangle { color: "#1E293B"; radius: 6; border.color: "#334155" }
        }
    }

    footer: DialogButtonBox {
        background: Rectangle { color: "#1E293B"; radius: 12 }
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
            background: Rectangle { radius: 6; color: "#334155" }
            contentItem: Text { text: "Kapat"; color: "white"; horizontalAlignment: Text.AlignHCenter }
        }
    }
}
