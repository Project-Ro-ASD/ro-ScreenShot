import QtQuick
import QtQuick.Controls

// Single premium button. variant: "primary" | "secondary" | "ghost" | "danger" | "dangerGhost"
Button {
    id: control
    opacity: enabled ? 1 : RoTokens.opacityDisabled
    RoMotion { id: motion }
    property string variant: "secondary"
    property var colors: RoPalette
    property int radiusValue: RoTokens.radiusMd

    leftPadding: RoTokens.spaceMd
    rightPadding: RoTokens.spaceMd
    implicitHeight: 38
    hoverEnabled: true
    focusPolicy: Qt.StrongFocus
    scale: control.down && RoMotionScale.factor > 0 ? motion.pressedScale : 1.0

    Behavior on scale {
        NumberAnimation { duration: motion.fast; easing.type: Easing.OutCubic }
    }

    readonly property color _bg: {
        if (!colors)
            return variant === "primary" ? RoPalette.accent : RoPalette.cardHover;
        if (variant === "primary")
            return hovered ? colors.accentHover : colors.accent;
        if (variant === "danger")
            return hovered ? RoPalette.danger : colors.danger;
        if (variant === "ghost" || variant === "dangerGhost")
            return hovered ? colors.cardHover : "transparent";
        return hovered ? colors.cardHover : colors.card;
    }
    readonly property color _fg: {
        if (!colors)
            return (variant === "primary" || variant === "danger") ? RoPalette.selectionText : RoPalette.text;
        if (variant === "danger")
            return RoPalette.dangerText;
        if (variant === "primary")
            return RoPalette.selectionText;
        if (variant === "dangerGhost")
            return colors.danger;
        return colors.text;
    }
    readonly property color _bd: {
        if (!colors)
            return (variant === "primary" || variant === "danger") ? "transparent" : RoPalette.border;
        if (variant === "primary" || variant === "danger")
            return "transparent";
        if (variant === "dangerGhost")
            return colors.danger;
        return colors.border;
    }

    contentItem: Text {
        text: control.text
        color: control._fg
        font.pixelSize: 13
        font.bold: variant === "primary" || variant === "danger"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    background: Rectangle {
        radius: control.radiusValue
        color: control._bg
        border.width: control.activeFocus ? 2 : ((control.variant === "secondary" || control.variant === "ghost" || control.variant === "dangerGhost") ? 1 : 0)
        border.color: control.activeFocus && control.colors ? control.colors.accent : control._bd
    }
}
