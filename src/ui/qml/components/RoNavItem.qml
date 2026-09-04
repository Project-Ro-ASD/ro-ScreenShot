import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Sidebar navigation item. Clean: icon + label + optional count.
Rectangle {
    id: root
    property string label: ""
    property string iconSource: ""
    property bool active: false
    property string badge: ""
    property var colors: null
    property bool compact: false
    signal clicked()

    height: 40
    radius: 8
    color: {
        if (!colors) return active ? "#2563EB" : "transparent"
        if (active) return colors.accent
        if (navMouse.containsMouse) return colors.cardHover
        return "transparent"
    }

    MouseArea {
        id: navMouse
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }

    ToolTip.visible: compact && navMouse.containsMouse
    ToolTip.delay: 300
    ToolTip.text: root.label

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: compact ? 0 : 12
        anchors.rightMargin: compact ? 0 : 12
        spacing: compact ? 0 : 10

        Item {
            Layout.preferredWidth: 20
            Layout.preferredHeight: 20
            Layout.alignment: compact ? Qt.AlignHCenter : Qt.AlignVCenter
            Image {
                anchors.centerIn: parent
                source: root.iconSource
                sourceSize.width: 18
                sourceSize.height: 18
                opacity: root.active ? 1.0 : 0.75
            }
        }
        Text {
            text: root.label
            visible: !compact
            color: root.active ? "#FFFFFF" : (root.colors ? root.colors.textMuted : "#4B5563")
            font.pixelSize: 13
            font.bold: root.active
            font.weight: root.active ? Font.DemiBold : Font.Normal
            Layout.fillWidth: true
            elide: Text.ElideRight
        }
        Rectangle {
            visible: !compact && root.badge.length > 0
            implicitWidth: Math.max(22, badgeText.implicitWidth + 12)
            height: 20
            radius: 10
            color: root.active ? "#1D4ED8" : (root.colors ? root.colors.cardHover : "#F2F4F7")
            Text {
                id: badgeText
                anchors.centerIn: parent
                text: root.badge
                color: root.active ? "#FFFFFF" : (root.colors ? root.colors.textSoft : "#6B7280")
                font.pixelSize: 11
                font.bold: true
            }
        }
    }
}
