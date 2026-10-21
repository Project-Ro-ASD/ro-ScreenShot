import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Sidebar navigation item. Clean: icon + label + optional count.
Rectangle {
    id: root
    opacity: enabled ? 1 : RoTokens.opacityDisabled
    activeFocusOnTab: true
    Accessible.role: Accessible.Button
    Accessible.name: root.label
    Keys.onSpacePressed: clicked()
    Keys.onReturnPressed: clicked()
    Keys.onEnterPressed: clicked()
    property string label: ""
    property string iconSource: ""
    property bool active: false
    property string badge: ""
    property var colors: RoPalette
    property bool compact: false
    signal clicked()

    height: 40
    radius: RoTokens.radiusSm
    color: {
        if (!colors) return active ? RoPalette.accent : "transparent"
        if (active) return colors.accent
        if (navMouse.containsMouse) return colors.cardHover
        return "transparent"
    }

    MouseArea {
        id: navMouse
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: { root.forceActiveFocus(); root.clicked(); }
    }

    ToolTip.visible: compact && navMouse.containsMouse
    ToolTip.delay: 300
    ToolTip.text: root.label

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
        anchors.leftMargin: compact ? 0 : 12
        anchors.rightMargin: compact ? 0 : 12
        spacing: compact ? 0 : 10

        Item {
            Layout.preferredWidth: 20
            Layout.preferredHeight: 20
            Layout.alignment: compact ? Qt.AlignHCenter : Qt.AlignVCenter
            RoIcon {
                color: root.active ? RoPalette.selectionText : RoPalette.textMuted
                anchors.centerIn: parent
                iconSource: root.iconSource
                sourceSize.width: 18
                sourceSize.height: 18
                opacity: root.active ? 1.0 : 0.75
            }
        }
        Text {
            text: root.label
            visible: !compact
            color: root.active ? RoPalette.selectionText : (root.colors ? root.colors.textMuted : RoPalette.textMuted)
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
            color: root.active ? (root.colors ? root.colors.accentHover : RoPalette.accentHover) : (root.colors ? root.colors.cardHover : RoPalette.cardHover)
            Text {
                id: badgeText
                anchors.centerIn: parent
                text: root.badge
                color: root.active ? RoPalette.selectionText : (root.colors ? root.colors.textSoft : RoPalette.textMuted)
                font.pixelSize: 11
                font.bold: true
            }
        }
    }
}
