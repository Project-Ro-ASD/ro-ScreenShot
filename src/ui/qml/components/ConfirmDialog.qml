import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

Item {
    id: dialogRoot

    property string title: qsTr("Onay")
    property string message: ""
    property string confirmText: qsTr("Onayla")
    property string cancelText: qsTr("İptal")
    property bool isDestructive: false
    property var colors: null

    signal accepted()
    signal rejected()

    anchors.fill: parent
    visible: opacity > 0.0
    opacity: 0.0

    Behavior on opacity {
        NumberAnimation { duration: 150 }
    }

    function open(dialogTitle, dialogMessage, destructive, onConfirm) {
        if (dialogTitle !== undefined) title = dialogTitle
        if (dialogMessage !== undefined) message = dialogMessage
        if (destructive !== undefined) isDestructive = destructive
        if (onConfirm) {
            var handler = function() {
                dialogRoot.accepted.disconnect(handler)
                onConfirm()
            }
            dialogRoot.accepted.connect(handler)
        }
        opacity = 1.0
        confirmBtn.forceActiveFocus()
    }

    function close() {
        opacity = 0.0
    }

    // Modal background dimmer
    Rectangle {
        anchors.fill: parent
        color: "#000000"
        opacity: 0.65

        MouseArea {
            anchors.fill: parent
            onClicked: {
                dialogRoot.rejected()
                dialogRoot.close()
            }
        }
    }

    // Dialog card
    Rectangle {
        id: card
        width: Math.min(dialogRoot.width - 40, 420)
        implicitHeight: contentCol.implicitHeight + 40
        anchors.centerIn: parent
        radius: 12
        color: (dialogRoot.colors && dialogRoot.colors.card) ? dialogRoot.colors.card : "#131D31"
        border.color: (dialogRoot.colors && dialogRoot.colors.border) ? dialogRoot.colors.border : "#334155"
        border.width: 1

        MouseArea {
            anchors.fill: parent
            preventStealing: true
        }

        ColumnLayout {
            id: contentCol
            anchors.fill: parent
            anchors.margins: 20
            spacing: 16

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Rectangle {
                    width: 32
                    height: 32
                    radius: 8
                    color: dialogRoot.isDestructive ? "#FEE2E2" : "#EFF4FF"

                    Text {
                        anchors.centerIn: parent
                        text: dialogRoot.isDestructive ? "!" : "i"
                        color: dialogRoot.isDestructive ? "#DC2626" : "#2563EB"
                        font.pixelSize: 16
                        font.bold: true
                    }
                }

                Text {
                    text: dialogRoot.title
                    color: (dialogRoot.colors && dialogRoot.colors.text) ? dialogRoot.colors.text : "#F8FAFC"
                    font.pixelSize: 16
                    font.bold: true
                    Layout.fillWidth: true
                }
            }

            Text {
                text: dialogRoot.message
                color: (dialogRoot.colors && dialogRoot.colors.textMuted) ? dialogRoot.colors.textMuted : "#CBD5E1"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Item { Layout.preferredHeight: 4 }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Item { Layout.fillWidth: true }

                Button {
                    text: dialogRoot.cancelText
                    focusPolicy: Qt.StrongFocus
                    onClicked: {
                        dialogRoot.rejected()
                        dialogRoot.close()
                    }
                    contentItem: Text {
                        text: parent.text
                        color: (dialogRoot.colors && dialogRoot.colors.textSoft) ? dialogRoot.colors.textSoft : "#94A3B8"
                        font.pixelSize: 13
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        implicitWidth: 90
                        implicitHeight: 36
                        color: parent.hovered ? (dialogRoot.colors ? dialogRoot.colors.border : "#334155") : "transparent"
                        border.color: (dialogRoot.colors && dialogRoot.colors.border) ? dialogRoot.colors.border : "#334155"
                        radius: 6
                    }
                }

                Button {
                    id: confirmBtn
                    text: dialogRoot.confirmText
                    focusPolicy: Qt.StrongFocus
                    onClicked: {
                        dialogRoot.accepted()
                        dialogRoot.close()
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "#FFFFFF"
                        font.pixelSize: 13
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        implicitWidth: 110
                        implicitHeight: 36
                        color: dialogRoot.isDestructive ? (parent.hovered ? "#DC2626" : "#EF4444")
                                                        : (parent.hovered ? "#1D4ED8" : "#2563EB")
                        radius: 6
                    }
                }
            }
        }
    }

    Keys.onEscapePressed: {
        dialogRoot.rejected()
        dialogRoot.close()
    }
    Keys.onReturnPressed: {
        dialogRoot.accepted()
        dialogRoot.close()
    }
}
