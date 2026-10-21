import QtQuick
import QtQuick.Controls
import QtQuick.Effects

Button {
    id: control
    opacity: enabled ? 1 : RoTokens.opacityDisabled
    RoMotion { id: motion }

    property string iconSource: ""
    property int iconSize: 18
    property color iconColor: colors ? (colors.textSoft || RoPalette.textMuted) : RoPalette.textMuted
    property color hoverColor: colors ? (colors.cardStrong || RoPalette.cardStrong) : RoPalette.cardStrong
    property color pressedColor: colors ? (colors.border || RoPalette.border) : RoPalette.border
    property color focusBorderColor: colors ? (colors.accentB || RoPalette.accent) : RoPalette.accent
    property var colors: RoPalette
    property string tooltipText: ""
    property int buttonRadius: RoTokens.radiusSm

    implicitWidth: 36
    implicitHeight: 36
    focusPolicy: Qt.StrongFocus
    hoverEnabled: true

    Accessible.role: Accessible.Button
    Accessible.name: tooltipText.length > 0 ? tooltipText : text
    Accessible.description: tooltipText

    contentItem: Item {
        anchors.fill: parent
        RoIcon {
            color: control.iconColor
            anchors.centerIn: parent
            iconSource: control.iconSource.indexOf(":") > 0
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
    ToolTip.delay: motion.tooltipDelay
    ToolTip.timeout: motion.tooltipTimeout
}
