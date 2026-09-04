import QtQuick
import QtQuick.Layouts
import "."

// Clean studio tool row: icon, title, subtitle, chevron. No emoji.
Rectangle {
    id: root
    property string title: ""
    property string subtitle: ""
    property string iconSource: ""
    property var colors: null
    signal clicked()

    implicitHeight: 64
    radius: 10
    color: hoverMouse.containsMouse ? (colors ? colors.cardHover : "#F2F4F7") : (colors ? colors.card : "#FFFFFF")
    border.color: hoverMouse.containsMouse ? (colors ? colors.accentMuted : "#BFDBFE") : (colors ? colors.border : "#E6E8EC")
    border.width: 1

    MouseArea {
        id: hoverMouse
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 12

        Rectangle {
            Layout.preferredWidth: 34
            Layout.preferredHeight: 34
            radius: 8
            color: colors ? colors.accentSoft : "#EFF6FF"
            Image {
                anchors.centerIn: parent
                source: root.iconSource
                sourceSize.width: 17
                sourceSize.height: 17
                visible: root.iconSource.length > 0
            }
            Text {
                anchors.centerIn: parent
                visible: root.iconSource.length === 0
                text: root.title.length > 0 ? root.title.charAt(0).toUpperCase() : "•"
                color: colors ? colors.accent : "#2563EB"
                font.pixelSize: 14
                font.bold: true
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 1
            Text {
                text: root.title
                color: colors ? colors.text : "#111827"
                font.pixelSize: 13
                font.bold: true
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
            Text {
                text: root.subtitle
                color: colors ? colors.textSoft : "#6B7280"
                font.pixelSize: 11
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }
        Text {
            text: "›"
            color: hoverMouse.containsMouse ? (colors ? colors.accent : "#2563EB") : (colors ? colors.placeholder : "#9CA3AF")
            font.pixelSize: 18
        }
    }
}
