import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RoDialog {
    id: pdfDialog
    title: "PDF Raporu ve Kılavuz Üretici"
    modal: true
    width: 520
    height: 420
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    property int selectedCount: 0
    signal generateRequested(string title, string notes, string outputPath)

    header: Rectangle {
        height: 52
        color: pdfDialog.shellColor
        radius: pdfDialog.surfaceRadius

        Text {
            anchors.centerIn: parent
            text: "📑 PDF Raporu Oluştur (" + selectedCount + " Görsel)"
            color: pdfDialog.textColor
            font.pixelSize: 16
            font.bold: true
        }
    }

    contentItem: ColumnLayout {
        spacing: 14

        Text {
            text: "Rapor Başlığı:"
            color: pdfDialog.mutedTextColor
            font.pixelSize: 13
        }

        TextField {
            id: titleField
            Layout.fillWidth: true
            text: "ro-ScreenShot Hata & Arayüz Raporu"
            color: "white"
            background: Rectangle { color: pdfDialog.shellColor; radius: 8; border.color: pdfDialog.borderColor }
        }

        Text {
            text: "Açıklama / Notlar:"
            color: pdfDialog.mutedTextColor
            font.pixelSize: 13
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 90
            color: pdfDialog.shellColor
            radius: 6
            border.color: pdfDialog.borderColor

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
            color: pdfDialog.mutedTextColor
            font.pixelSize: 13
        }

        TextField {
            id: pathField
            Layout.fillWidth: true
            text: "/tmp/ro_screenshot_report.pdf"
            color: "white"
            background: Rectangle { color: pdfDialog.shellColor; radius: 8; border.color: pdfDialog.borderColor }
        }
    }

    footer: DialogButtonBox {
        background: Rectangle { color: pdfDialog.shellColor; radius: pdfDialog.surfaceRadius }
        Button {
            text: "İptal"
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
            background: Rectangle { radius: 8; color: pdfDialog.elevatedColor }
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
