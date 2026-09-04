import QtQuick
import QtQuick.Layouts

// Capture card: icon badge + title + description + shortcut chip.
Rectangle {
    id: root
    property string title: ""
    property string description: ""
    property string shortcut: ""
    property string iconSource: ""
    property var colors: null
    signal clicked()

    implicitHeight: 118
    radius: 12
    color: cardMouse.containsMouse ? (colors ? colors.cardHover : "#F2F4F7") : (colors ? colors.card : "#FFFFFF")
    border.color: cardMouse.containsMouse ? (colors ? colors.accent : "#2563EB") : (colors ? colors.border : "#E6E8EC")
    border.width: cardMouse.containsMouse ? 1.5 : 1

    MouseArea {
        id: cardMouse
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 14

        Rectangle {
            Layout.preferredWidth: 44
            Layout.preferredHeight: 44
            Layout.alignment: Qt.AlignTop
            radius: 10
            color: colors ? colors.accentSoft : "#EFF6FF"
            Image {
                anchors.centerIn: parent
                source: root.iconSource
                sourceSize.width: 22
                sourceSize.height: 22
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            Text {
                text: root.title
                color: colors ? colors.text : "#111827"
                font.pixelSize: 14
                font.bold: true
                font.weight: Font.DemiBold
            }
            Text {
                text: root.description
                color: colors ? colors.textSoft : "#6B7280"
                font.pixelSize: 12
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                maximumLineCount: 2
                elide: Text.ElideRight
            }
            Item { height: 2 }
            Rectangle {
                visible: root.shortcut.length > 0
                implicitWidth: chipText.implicitWidth + 14
                height: 20
                radius: 5
                color: colors ? colors.codeBg : "#F2F4F7"
                border.color: colors ? colors.border : "#E6E8EC"
                border.width: 1
                Text {
                    id: chipText
                    anchors.centerIn: parent
                    text: root.shortcut
                    color: colors ? colors.textMuted : "#4B5563"
                    font.pixelSize: 10
                    font.bold: true
                    font.family: "monospace"
                }
            }
        }
    }
}
