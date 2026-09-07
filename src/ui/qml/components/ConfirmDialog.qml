import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

Item {
    id: dialogRoot
    RoMotion { id: motion }

    property string title: qsTr("Onay")
    property string message: ""
    property string confirmText: qsTr("Onayla")
    property string cancelText: qsTr("İptal")
    property bool isDestructive: false
    property var colors: null

    signal accepted
    signal rejected

    anchors.fill: parent
    visible: opacity > 0.0
    opacity: 0.0

    Behavior on opacity {
        NumberAnimation {
            duration: motion.standard
            easing.type: Easing.OutCubic
        }
    }

    function open(dialogTitle, dialogMessage, destructive, onConfirm) {
        if (dialogTitle !== undefined)
            title = dialogTitle;
        if (dialogMessage !== undefined)
            message = dialogMessage;
        if (destructive !== undefined)
            isDestructive = destructive;
        if (onConfirm) {
            var handler = function () {
                dialogRoot.accepted.disconnect(handler);
                onConfirm();
            };
            dialogRoot.accepted.connect(handler);
        }
        opacity = 1.0;
        confirmBtn.forceActiveFocus();
    }

    function close() {
        opacity = 0.0;
    }

    // Modal background dimmer
    Rectangle {
        anchors.fill: parent
        color: "#000000"
        opacity: 0.65

        MouseArea {
            anchors.fill: parent
            onClicked: {
                dialogRoot.rejected();
                dialogRoot.close();
            }
        }
    }

    // Dialog card
    Rectangle {
        id: card
        scale: dialogRoot.opacity > 0 ? 1.0 : motion.dialogHiddenScale
        width: Math.min(dialogRoot.width - 40, 420)
        implicitHeight: contentCol.implicitHeight + 40
        anchors.centerIn: parent
        radius: 18
        color: (dialogRoot.colors && dialogRoot.colors.card) ? dialogRoot.colors.card : "#1B1F2A"
        border.color: (dialogRoot.colors && dialogRoot.colors.border) ? dialogRoot.colors.border : "#454D60"
        border.width: 1
        Behavior on scale { NumberAnimation { duration: motion.standard; easing.type: Easing.OutBack } }

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
                    radius: 10
                    color: dialogRoot.isDestructive ? "#F6E7EB" : (dialogRoot.colors ? dialogRoot.colors.accentSoft : "#ECECFF")

                    Text {
                        anchors.centerIn: parent
                        text: dialogRoot.isDestructive ? "!" : "i"
                        color: dialogRoot.isDestructive ? "#C4475D" : (dialogRoot.colors ? dialogRoot.colors.accent : "#5A5FE8")
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

            Item {
                Layout.preferredHeight: 4
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    text: dialogRoot.cancelText
                    focusPolicy: Qt.StrongFocus
                    onClicked: {
                        dialogRoot.rejected();
                        dialogRoot.close();
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
                        color: parent.hovered ? (dialogRoot.colors ? dialogRoot.colors.cardHover : "#303746") : "transparent"
                        border.color: (dialogRoot.colors && dialogRoot.colors.border) ? dialogRoot.colors.border : "#454D60"
                        radius: 9
                    }
                }

                Button {
                    id: confirmBtn
                    text: dialogRoot.confirmText
                    focusPolicy: Qt.StrongFocus
                    onClicked: {
                        dialogRoot.accepted();
                        dialogRoot.close();
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
                        color: dialogRoot.isDestructive ? (parent.hovered ? "#A7344B" : "#C4475D") : (parent.hovered ? "#474BCE" : "#5A5FE8")
                        radius: 9
                    }
                }
            }
        }
    }

    Keys.onEscapePressed: {
        dialogRoot.rejected();
        dialogRoot.close();
    }
    Keys.onReturnPressed: {
        dialogRoot.accepted();
        dialogRoot.close();
    }
}
