import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RoDialog {
    id: vaultDialog
    title: "🔒 Şifreli Gizlilik Kasası (AES-256)"
    modal: true
    width: 600
    height: 480
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    property bool isUnlocked: false
    property string statusText: ""

    signal unlockRequested(string password)
    signal lockRequested()

    header: Rectangle {
        height: 52
        color: vaultDialog.shellColor
        radius: vaultDialog.surfaceRadius

        Text {
            anchors.centerIn: parent
            text: "🔒 Şifreli Gizlilik Kasası (AES-256)"
            color: vaultDialog.textColor
            font.pixelSize: 16
            font.bold: true
        }
    }

    contentItem: ColumnLayout {
        spacing: 16

        Text {
            text: isUnlocked ? "🔓 Kasa Açık (Şifrelenmiş Ekran Görüntüleri)" : "🔒 Kasayı Açmak İçin Parolanızı Girin:"
            color: isUnlocked ? "#10B981" : "#94A3B8"
            font.bold: true
            font.pixelSize: 14
        }

        TextField {
            id: passField
            visible: !isUnlocked
            Layout.fillWidth: true
            echoMode: TextInput.Password
            placeholderText: "Kasa Parolası..."
            color: "white"
            background: Rectangle { color: vaultDialog.shellColor; radius: 8; border.color: vaultDialog.borderColor }
            onAccepted: {
                vaultDialog.unlockRequested(passField.text);
            }
        }

        Text {
            visible: statusText !== ""
            text: statusText
            color: "#EF4444"
            font.pixelSize: 12
        }

        Rectangle {
            visible: isUnlocked
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: vaultDialog.shellColor
            radius: 8

            Text {
                anchors.centerIn: parent
                text: "Kasa aktif ve güvenli. Seçilen görseller şifrelenerek saklanmaktadır."
                color: vaultDialog.mutedTextColor
            }
        }
    }

    footer: DialogButtonBox {
        background: Rectangle { color: vaultDialog.shellColor; radius: vaultDialog.surfaceRadius }
        Button {
            text: isUnlocked ? "Kasayı Kilitle" : "Kilidi Aç"
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            background: Rectangle { radius: 6; color: isUnlocked ? "#EF4444" : "#3B82F6" }
            contentItem: Text { text: isUnlocked ? "Kasayı Kilitle" : "Kilidi Aç"; color: "white"; font.bold: true; horizontalAlignment: Text.AlignHCenter }
            onClicked: {
                if (isUnlocked) {
                    vaultDialog.lockRequested();
                } else {
                    vaultDialog.unlockRequested(passField.text);
                }
            }
        }
        Button {
            text: "Kapat"
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
            background: Rectangle { radius: 8; color: vaultDialog.elevatedColor }
            contentItem: Text { text: "Kapat"; color: "white"; horizontalAlignment: Text.AlignHCenter }
        }
    }
}
