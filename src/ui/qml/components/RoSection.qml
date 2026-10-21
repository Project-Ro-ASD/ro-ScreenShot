import QtQuick
import QtQuick.Layouts

// Settings section card with icon + title. Content via default slot.
Rectangle {
    id: root
    property string title: ""
    property string iconSource: ""
    property var colors: RoPalette
    default property alias content: body.data

    radius: RoTokens.radiusMd
    color: colors ? colors.card : RoPalette.card
    border.color: colors ? colors.border : RoPalette.border
    border.width: 1
    implicitHeight: layout.implicitHeight + 32

    ColumnLayout {
        id: layout
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 16
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            Rectangle {
                Layout.preferredWidth: 28
                Layout.preferredHeight: 28
                radius: RoTokens.radiusSm
                color: root.colors ? root.colors.accentSoft : RoPalette.accentSoft
                RoIcon {
                    anchors.centerIn: parent
                    iconSource: root.iconSource
                    sourceSize.width: 15
                    sourceSize.height: 15
                    visible: root.iconSource.length > 0
                }
            }
            Text {
                text: root.title
                color: root.colors ? root.colors.text : RoPalette.text

                font.pixelSize: 14
                font.bold: true
                font.weight: Font.DemiBold
                Layout.fillWidth: true
            }
        }

        ColumnLayout {
            id: body
            Layout.fillWidth: true
            spacing: 10
        }
    }
}
