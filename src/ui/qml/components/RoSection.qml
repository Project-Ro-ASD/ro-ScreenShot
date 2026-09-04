import QtQuick
import QtQuick.Layouts

// Settings section card with icon + title. Content via default slot.
Rectangle {
    id: root
    property string title: ""
    property string iconSource: ""
    property var colors: null
    default property alias content: body.data

    radius: 12
    color: colors ? colors.card : "#FFFFFF"
    border.color: colors ? colors.border : "#E6E8EC"
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
                radius: 7
                color: root.colors ? root.colors.accentSoft : "#EFF6FF"
                Image {
                    anchors.centerIn: parent
                    source: root.iconSource
                    sourceSize.width: 15
                    sourceSize.height: 15
                    visible: root.iconSource.length > 0
                }
            }
            Text {
                text: root.title
                color: root.colors ? root.colors.text : "#111827"
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
