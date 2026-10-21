import QtQuick
import QtQuick.Layouts

// Page header: title + subtitle, no hero card. Premium minimal.
ColumnLayout {
    id: root
    property string title: ""
    property string subtitle: ""
    property string statusText: ""
    property bool statusActive: true
    property var colors: RoPalette

    spacing: 2

    RowLayout {
        Layout.fillWidth: true
        spacing: 10

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2
            Text {
                text: root.title
                color: root.colors ? root.colors.text : RoPalette.text

                font.pixelSize: 20
                font.bold: true
                font.weight: Font.DemiBold
            }
            Text {
                text: root.subtitle
                color: root.colors ? root.colors.textSoft : RoPalette.textMuted

                font.pixelSize: 13
                visible: root.subtitle.length > 0
            }
        }

        Rectangle {
            visible: root.statusText.length > 0
            height: 26
            implicitWidth: statusRow.implicitWidth + 18
            radius: 13
            color: root.colors ? root.colors.card : RoPalette.card
            border.color: root.colors ? root.colors.border : RoPalette.border
            border.width: 1
            RowLayout {
                id: statusRow
                anchors.centerIn: parent
                spacing: 6
                Rectangle {
                    width: 7; height: 7; radius: 3.5
                    color: root.statusActive ? (root.colors ? root.colors.success : RoPalette.success) : (root.colors ? root.colors.muted : RoPalette.placeholder)
                }
                Text {
                    text: root.statusText
                    color: root.colors ? root.colors.textMuted : RoPalette.textMuted

                    font.pixelSize: 11
                    font.bold: true
                }
            }
        }
    }
}
