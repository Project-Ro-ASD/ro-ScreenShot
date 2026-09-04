import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: pdfDialog
    title: "PDF Raporu ve Kılavuz Üretici"
    modal: true
    width: 520
    height: 420
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    property int selectedCount: 0
    signal generateRequested(string title, string notes, string outputPath)

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
            text: "📑 PDF Raporu Oluştur (" + selectedCount + " Görsel)"
            color: "#F8FAFC"
            font.pixelSize: 16
            font.bold: true
        }
    }

    contentItem: ColumnLayout {
        spacing: 14

        Text {
            text: "Rapor Başlığı:"
            color: "#94A3B8"
            font.pixelSize: 13
        }

        TextField {
            id: titleField
            Layout.fillWidth: true
            text: "ro-ScreenShot Hata & Arayüz Raporu"
            color: "white"
            background: Rectangle { color: "#1E293B"; radius: 6; border.color: "#334155" }
        }

        Text {
            text: "Açıklama / Notlar:"
            color: "#94A3B8"
            font.pixelSize: 13
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 90
            color: "#1E293B"
            radius: 6
            border.color: "#334155"

            ScrollView {
                anchors.fill: parent
                anchors.margins: 6
                clip: true

                TextEdit {
                    id: notesField
                    color: "white"
                    font.pixelSize: 12
                    wrapMode: TextEdit.Wrap
                    selectByMouse: true
                }
            }
        }

        Text {
            text: "Kayıt Dosyası:"
            color: "#94A3B8"
            font.pixelSize: 13
        }

        TextField {
            id: pathField
            Layout.fillWidth: true
            text: "/tmp/ro_screenshot_report.pdf"
            color: "white"
            background: Rectangle { color: "#1E293B"; radius: 6; border.color: "#334155" }
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
            text: "PDF Oluştur"
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            background: Rectangle { radius: 6; color: "#3B82F6" }
            contentItem: Text { text: "PDF Oluştur"; color: "white"; font.bold: true; horizontalAlignment: Text.AlignHCenter }
            onClicked: {
                pdfDialog.generateRequested(titleField.text, notesField.text, pathField.text);
                pdfDialog.accept();
            }
        }
    }
}
