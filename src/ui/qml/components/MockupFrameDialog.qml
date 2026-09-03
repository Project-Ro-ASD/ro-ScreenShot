import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: mockupDialog
    title: "Görsel Giydirme & Mockup Çerçevesi"
    modal: true
    width: 640
    height: 520
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    property int itemRow: -1
    property int selectedPreset: 0
    property int framePadding: 48
    signal exportRequested(int row, int preset, int padding)

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
            text: "🎨 Sosyal Medya & Mockup Çerçevesi"
            color: "#F8FAFC"
            font.pixelSize: 16
            font.bold: true
        }
    }

    contentItem: ColumnLayout {
        spacing: 16

        Text {
            text: "Arka Plan Gradyanı Seçin:"
            color: "#94A3B8"
            font.pixelSize: 13
        }

        RowLayout {
            spacing: 12

            Repeater {
                model: [
                    { name: "Sunset", c1: "#FF512F", c2: "#DD2476", idx: 0 },
                    { name: "Ocean", c1: "#2193b0", c2: "#6dd5ed", idx: 1 },
                    { name: "Cyberpunk", c1: "#8A2387", c2: "#F27121", idx: 2 },
                    { name: "Velvet", c1: "#1e130c", c2: "#9a8478", idx: 3 },
                    { name: "Emerald", c1: "#0ba360", c2: "#3cba92", idx: 4 },
                    { name: "Slate", c1: "#1e293b", c2: "#0f172a", idx: 5 }
                ]

                Rectangle {
                    width: 56
                    height: 56
                    radius: 28
                    border.width: selectedPreset === modelData.idx ? 3 : 1
                    border.color: selectedPreset === modelData.idx ? "#38BDF8" : "#475569"

                    gradient: Gradient {
                        GradientStop { position: 0.0; color: modelData.c1 }
                        GradientStop { position: 1.0; color: modelData.c2 }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: selectedPreset = modelData.idx
                    }
                }
            }
        }

        Text {
            text: "Kenar Boşluğu (Padding): " + paddingSlider.value.toFixed(0) + "px"
            color: "#94A3B8"
            font.pixelSize: 13
        }

        Slider {
            id: paddingSlider
            Layout.fillWidth: true
            from: 20
            to: 100
            value: 48
            stepSize: 4
            onValueChanged: framePadding = value
        }
    }

    footer: DialogButtonBox {
        background: Rectangle { color: "#1E293B"; radius: 12 }
        Button {
            text: "İptal"
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
            background: Rectangle { radius: 6; color: "#334155" }
            contentItem: Text { text: "İptal"; color: "white"; horizontalAlignment: Text.AlignHCenter }
        }
        Button {
            text: "Mockup Olarak Kaydet"
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            background: Rectangle { radius: 6; color: "#3B82F6" }
            contentItem: Text { text: "Mockup Olarak Kaydet"; color: "white"; font.bold: true; horizontalAlignment: Text.AlignHCenter }
            onClicked: {
                mockupDialog.exportRequested(itemRow, selectedPreset, framePadding);
                mockupDialog.accept();
            }
        }
    }
}
