import QtQuick
import QtQuick.Layouts
import "."

// Clean studio tool row: icon, title, subtitle, chevron. No emoji.
Rectangle {
    id: root
    opacity: enabled ? 1 : RoTokens.opacityDisabled
    activeFocusOnTab: true
    Accessible.role: Accessible.Button
    Accessible.name: root.title
    Keys.onSpacePressed: clicked()
    Keys.onReturnPressed: clicked()
    Keys.onEnterPressed: clicked()
    property string title: ""
    property string subtitle: ""
    property string iconSource: ""
    property var colors: RoPalette
    signal clicked()

    implicitHeight: 64
    radius: RoTokens.radiusMd
    color: hoverMouse.containsMouse ? (colors ? colors.cardHover : RoPalette.cardHover) : (colors ? colors.card : RoPalette.card)
    border.color: hoverMouse.containsMouse ? (colors ? colors.accentMuted : RoPalette.accentMuted) : (colors ? colors.border : RoPalette.border)
    border.width: 1

    MouseArea {
        id: hoverMouse
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: { root.forceActiveFocus(); root.clicked(); }
    }

    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        color: "transparent"
        border.width: RoTokens.focusRingWidth
        border.color: RoPalette.accent
        visible: root.activeFocus
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 12

        Rectangle {
            Layout.preferredWidth: 34
            Layout.preferredHeight: 34
            radius: RoTokens.radiusSm
            color: colors ? colors.accentSoft : RoPalette.accentSoft
            RoIcon {
                anchors.centerIn: parent
                iconSource: root.iconSource
                sourceSize.width: 17
                sourceSize.height: 17
                visible: root.iconSource.length > 0
            }
            Text {
                anchors.centerIn: parent
                visible: root.iconSource.length === 0
                text: root.title.length > 0 ? root.title.charAt(0).toUpperCase() : "•"
                color: colors ? colors.accent : RoPalette.accent

                font.pixelSize: 14
                font.bold: true
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 1
            Text {
                text: root.title
                color: colors ? colors.text : RoPalette.text

                font.pixelSize: 13
                font.bold: true
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
            Text {
                text: root.subtitle
                color: colors ? colors.textSoft : RoPalette.textMuted

                font.pixelSize: 11
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }
        Text {
            text: "›"
            color: hoverMouse.containsMouse ? (colors ? colors.accent : RoPalette.accent) : (colors ? colors.placeholder : RoPalette.placeholder)
            font.pixelSize: 18
        }
    }
}
