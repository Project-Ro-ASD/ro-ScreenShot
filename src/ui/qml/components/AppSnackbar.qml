import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

Item {
    id: snackbarRoot

    property var colors: null
    property var messageQueue: []
    property bool isShowing: false
    property string currentText: ""
    property string currentType: "info" // "success", "error", "warning", "info"
    property string currentActionText: ""
    property var currentActionCallback: null

    implicitWidth: 380
    implicitHeight: 48
    anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined
    anchors.bottom: parent ? parent.bottom : undefined
    anchors.bottomMargin: 24
    z: 9999

    function showMessage(text, type, actionText, actionCallback) {
        var item = {
            text: text,
            type: type || "info",
            actionText: actionText || "",
            actionCallback: actionCallback || null
        }
        messageQueue.push(item)
        if (!isShowing) {
            processNextMessage()
        }
    }

    function processNextMessage() {
        if (messageQueue.length === 0) {
            isShowing = false
            return
        }
        var next = messageQueue.shift()
        currentText = next.text
        currentType = next.type
        currentActionText = next.actionText
        currentActionCallback = next.actionCallback
        isShowing = true
        dismissTimer.restart()
    }

    function dismiss() {
        isShowing = false
        dismissTimer.stop()
        queueTimer.restart()
    }

    Timer {
        id: dismissTimer
        interval: currentActionText.length > 0 ? 5500 : 3500
        repeat: false
        onTriggered: snackbarRoot.dismiss()
    }

    Timer {
        id: queueTimer
        interval: 200
        repeat: false
        onTriggered: snackbarRoot.processNextMessage()
    }

    Rectangle {
        id: bg
        anchors.fill: parent
        radius: 8
        color: (snackbarRoot.colors && snackbarRoot.colors.cardStrong) ? snackbarRoot.colors.cardStrong : "#1E293B"
        border.color: {
            if (currentType === "success") return (snackbarRoot.colors && snackbarRoot.colors.success) ? snackbarRoot.colors.success : "#10B981"
            if (currentType === "error") return (snackbarRoot.colors && snackbarRoot.colors.danger) ? snackbarRoot.colors.danger : "#EF4444"
            if (currentType === "warning") return (snackbarRoot.colors && snackbarRoot.colors.warning) ? snackbarRoot.colors.warning : "#F59E0B"
            return (snackbarRoot.colors && snackbarRoot.colors.accentB) ? snackbarRoot.colors.accentB : "#3B82F6"
        }
        border.width: 1.5
        opacity: snackbarRoot.isShowing ? 1.0 : 0.0
        y: snackbarRoot.isShowing ? 0 : 20

        Behavior on opacity { NumberAnimation { duration: 180 } }
        Behavior on y { NumberAnimation { duration: 180; easing.type: Easing.OutCubic } }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 14
            anchors.rightMargin: 10
            spacing: 12

            Text {
                text: snackbarRoot.currentText
                color: (snackbarRoot.colors && snackbarRoot.colors.text) ? snackbarRoot.colors.text : "#F8FAFC"
                font.pixelSize: 13
                font.bold: true
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            // Optional action button (e.g. Undo)
            Button {
                visible: snackbarRoot.currentActionText.length > 0
                text: snackbarRoot.currentActionText
                onClicked: {
                    if (snackbarRoot.currentActionCallback) {
                        snackbarRoot.currentActionCallback()
                    }
                    snackbarRoot.dismiss()
                }
                contentItem: Text {
                    text: parent.text
                    color: "#38BDF8"
                    font.pixelSize: 12
                    font.bold: true
                }
                background: Rectangle {
                    implicitWidth: 60
                    implicitHeight: 28
                    color: parent.hovered ? "#0F172A" : "transparent"
                    radius: 4
                }
            }

            // Close button
            Button {
                flat: true
                onClicked: snackbarRoot.dismiss()
                contentItem: Text {
                    text: "✕"
                    color: (snackbarRoot.colors && snackbarRoot.colors.textSoft) ? snackbarRoot.colors.textSoft : "#94A3B8"
                    font.pixelSize: 12
                }
                background: Item {
                    implicitWidth: 24
                    implicitHeight: 24
                }
            }
        }
    }
}
