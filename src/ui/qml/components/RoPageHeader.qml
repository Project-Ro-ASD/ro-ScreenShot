import QtQuick
import QtQuick.Layouts

// Page header: title + subtitle, no hero card. Premium minimal.
ColumnLayout {
    id: root
    property string title: ""
    property string subtitle: ""
    property string statusText: ""
    property bool statusActive: true
    property var colors: null

    spacing: 2

    RowLayout {
        Layout.fillWidth: true
        spacing: 10

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2
            Text {
                text: root.title
                color: root.colors ? root.colors.text : "#111827"
                font.pixelSize: 20
                font.bold: true
                font.weight: Font.DemiBold
            }
            Text {
                text: root.subtitle
                color: root.colors ? root.colors.textSoft : "#6B7280"
                font.pixelSize: 13
                visible: root.subtitle.length > 0
            }
        }

        Rectangle {
            visible: root.statusText.length > 0
            height: 26
            implicitWidth: statusRow.implicitWidth + 18
            radius: 13
            color: root.colors ? root.colors.card : "#FFFFFF"
            border.color: root.colors ? root.colors.border : "#E6E8EC"
            border.width: 1
            RowLayout {
                id: statusRow
                anchors.centerIn: parent
                spacing: 6
                Rectangle {
                    width: 7; height: 7; radius: 3.5
                    color: root.statusActive ? "#059669" : "#9CA3AF"
                }
                Text {
                    text: root.statusText
                    color: root.colors ? root.colors.textMuted : "#4B5563"
                    font.pixelSize: 11
                    font.bold: true
                }
            }
        }
    }
}
