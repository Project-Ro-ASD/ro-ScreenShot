import QtQuick
import QtQuick.Controls
import QtQuick.Effects

Button {
    id: control

    property string iconSource: ""
    property int iconSize: 18
    property color iconColor: colors ? (colors.textSoft || "#94A3B8") : "#94A3B8"
    property color hoverColor: colors ? (colors.cardStrong || "#1E293B") : "#1E293B"
    property color pressedColor: colors ? (colors.border || "#334155") : "#334155"
    property color focusBorderColor: colors ? (colors.accentB || "#3B82F6") : "#3B82F6"
    property var colors: null
    property string tooltipText: ""
    property int buttonRadius: 6

    implicitWidth: 36
    implicitHeight: 36
    focusPolicy: Qt.StrongFocus
    hoverEnabled: true

    Accessible.role: Accessible.Button
    Accessible.name: tooltipText.length > 0 ? tooltipText : text
    Accessible.description: tooltipText

    contentItem: Item {
        anchors.fill: parent
        Image {
            anchors.centerIn: parent
            source: control.iconSource.indexOf(":") > 0
                    ? control.iconSource
                    : (control.iconSource.indexOf("assets/") === 0
                       ? "../" + control.iconSource
                       : control.iconSource)
            sourceSize.width: control.iconSize
            sourceSize.height: control.iconSize
            width: control.iconSize
            height: control.iconSize
            visible: control.iconSource.length > 0
        }
    }

    background: Rectangle {
        implicitWidth: control.implicitWidth
        implicitHeight: control.implicitHeight
        radius: control.buttonRadius
        color: control.down ? control.pressedColor : (control.hovered ? control.hoverColor : "transparent")
        border.color: control.activeFocus ? control.focusBorderColor : (control.hovered && control.colors ? control.colors.border : "transparent")
        border.width: control.activeFocus ? 2 : 1
    }

    ToolTip.visible: control.hovered && tooltipText.length > 0
    ToolTip.text: control.tooltipText
    ToolTip.delay: 350
    ToolTip.timeout: 4000
}
