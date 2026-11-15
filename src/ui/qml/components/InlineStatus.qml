import QtQuick
import QtQuick.Layouts
import QtQuick.Effects

Item {
    id: root

    property string status: "info" // "success", "error", "warning", "info"
    property string text: ""
    property var colors: RoPalette
    property int textSize: 12

    implicitWidth: layout.implicitWidth
    implicitHeight: Math.max(22, layout.implicitHeight)
    visible: text.length > 0

    readonly property color statusColor: {
        if (status === "success") return (colors && colors.success) ? colors.success : RoPalette.success
        if (status === "error") return (colors && colors.danger) ? colors.danger : RoPalette.danger
        if (status === "warning") return (colors && colors.warning) ? colors.warning : RoPalette.warning
        return (colors && colors.accentB) ? colors.accentB : RoPalette.accent
    }

    readonly property string statusIconText: {
        if (status === "success") return "✓"
        if (status === "error") return "✕"
        if (status === "warning") return "!"
        return "ℹ"
    }

    RowLayout {
        id: layout
        anchors.fill: parent
        spacing: 8

        Rectangle {
            width: 16
            height: 16
            radius: RoTokens.radiusSm
            color: root.statusColor
            Layout.alignment: Qt.AlignVCenter

            Text {
                anchors.centerIn: parent
                text: root.statusIconText
                color: RoPalette.card
                font.pixelSize: 10
                font.bold: true
            }
        }

        Text {
            text: root.text
            color: (root.colors && root.colors.text) ? root.colors.text : RoPalette.text
            font.pixelSize: root.textSize
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
        }
    }
}
