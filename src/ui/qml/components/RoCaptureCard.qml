import QtQuick
import QtQuick.Layouts

// Capture card: icon badge + title + description + shortcut chip.
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
    property string description: ""
    property string shortcut: ""
    property string iconSource: ""
    property var colors: RoPalette
    signal clicked()

    implicitHeight: 118
    radius: RoTokens.radiusMd
    color: cardMouse.containsMouse ? (colors ? colors.cardHover : RoPalette.cardHover) : (colors ? colors.card : RoPalette.card)
    border.color: cardMouse.containsMouse ? (colors ? colors.accent : RoPalette.accent) : (colors ? colors.border : RoPalette.border)
    border.width: cardMouse.containsMouse ? 1.5 : 1

    MouseArea {
        id: cardMouse
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
        anchors.margins: 16
        spacing: 14

        Rectangle {
            Layout.preferredWidth: 44
            Layout.preferredHeight: 44
            Layout.alignment: Qt.AlignTop
            radius: RoTokens.radiusSm
            color: colors ? colors.accentSoft : RoPalette.accentSoft
            RoIcon {
                anchors.centerIn: parent
                iconSource: root.iconSource
                sourceSize.width: 22
                sourceSize.height: 22
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            Text {
                text: root.title
                color: colors ? colors.text : RoPalette.text

                font.pixelSize: 14
                font.bold: true
                font.weight: Font.DemiBold
            }
            Text {
                text: root.description
                color: colors ? colors.textSoft : RoPalette.textMuted

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
                color: colors ? colors.codeBg : RoPalette.cardHover
                border.color: colors ? colors.border : RoPalette.border
                border.width: 1
                Text {
                    id: chipText
                    anchors.centerIn: parent
                    text: root.shortcut
                    color: colors ? colors.textMuted : RoPalette.textMuted
                    font.pixelSize: 10
                    font.bold: true

                }
            }
        }
    }
}
