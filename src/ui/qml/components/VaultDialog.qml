import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
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
            text: "🔒 Şifreli Gizlilik Kasası (AES-256)"
            color: "#F8FAFC"
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
            background: Rectangle { color: "#1E293B"; radius: 6; border.color: "#334155" }
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
            color: "#1E293B"
            radius: 8

            Text {
                anchors.centerIn: parent
                text: "Kasa aktif ve güvenli. Seçilen görseller şifrelenerek saklanmaktadır."
                color: "#94A3B8"
            }
        }
    }

    footer: DialogButtonBox {
        background: Rectangle { color: "#1E293B"; radius: 12 }
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
            background: Rectangle { radius: 6; color: "#334155" }
            contentItem: Text { text: "Kapat"; color: "white"; horizontalAlignment: Text.AlignHCenter }
        }
    }
}
