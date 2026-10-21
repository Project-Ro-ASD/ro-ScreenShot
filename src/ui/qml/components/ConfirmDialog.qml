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
    property var colors: RoPalette

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
        color: RoPalette.scrim
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
        radius: RoTokens.radiusLg
        color: (dialogRoot.colors && dialogRoot.colors.card) ? dialogRoot.colors.card : RoPalette.card
        border.color: (dialogRoot.colors && dialogRoot.colors.border) ? dialogRoot.colors.border : RoPalette.border
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
                    color: dialogRoot.isDestructive ? RoPalette.dangerSoft : (dialogRoot.colors ? dialogRoot.colors.accentSoft : RoPalette.accentSoft)

                    Text {
                        anchors.centerIn: parent
                        text: dialogRoot.isDestructive ? "!" : "i"
                        color: dialogRoot.isDestructive ? RoPalette.danger : (dialogRoot.colors ? dialogRoot.colors.accent : RoPalette.accent)
                        font.pixelSize: 16
                        font.bold: true
                    }
                }

                Text {
                    text: dialogRoot.title
                    color: (dialogRoot.colors && dialogRoot.colors.text) ? dialogRoot.colors.text : RoPalette.text
                    font.pixelSize: 16
                    font.bold: true
                    Layout.fillWidth: true
                }
            }

            Text {
                text: dialogRoot.message
                color: (dialogRoot.colors && dialogRoot.colors.textMuted) ? dialogRoot.colors.textMuted : RoPalette.textMuted
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
                        color: (dialogRoot.colors && dialogRoot.colors.textSoft) ? dialogRoot.colors.textSoft : RoPalette.textMuted
                        font.pixelSize: 13
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        implicitWidth: 90
                        implicitHeight: 36
                        color: parent.hovered ? (dialogRoot.colors ? dialogRoot.colors.cardHover : RoPalette.cardHover) : "transparent"
                        border.color: (dialogRoot.colors && dialogRoot.colors.border) ? dialogRoot.colors.border : RoPalette.border
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
                        color: dialogRoot.isDestructive ? RoPalette.dangerText : RoPalette.selectionText
                        font.pixelSize: 13
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        implicitWidth: 110
                        implicitHeight: 36
                        color: dialogRoot.isDestructive ? (parent.hovered ? RoPalette.danger : RoPalette.danger) : (parent.hovered ? RoPalette.accentHover : RoPalette.accent)
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
