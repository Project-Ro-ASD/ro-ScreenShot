import QtQuick
import QtQuick.Controls

// Single premium button. variant: "primary" | "secondary" | "ghost" | "danger" | "dangerGhost"
Button {
    id: control
    RoMotion { id: motion }
    property string variant: "secondary"
    property var colors: null
    property int radiusValue: 8

    implicitHeight: 38
    hoverEnabled: true
    focusPolicy: Qt.StrongFocus
    scale: control.down ? motion.pressedScale : 1.0

    Behavior on scale {
        NumberAnimation { duration: motion.fast; easing.type: Easing.OutCubic }
    }

    readonly property color _bg: {
        if (!colors)
            return variant === "primary" ? "#4F46E5" : "#F1F5F9";
        if (variant === "primary")
            return hovered ? colors.accentHover : colors.accent;
        if (variant === "danger")
            return hovered ? "#A7344B" : colors.danger;
        if (variant === "ghost" || variant === "dangerGhost")
            return hovered ? colors.cardHover : "transparent";
        return hovered ? colors.cardHover : colors.card;
    }
    readonly property color _fg: {
        if (!colors)
            return "#111827";
        if (variant === "primary" || variant === "danger")
            return "#FFFFFF";
        if (variant === "dangerGhost")
            return colors.danger;
        return colors.text;
    }
    readonly property color _bd: {
        if (!colors)
            return "#E6E8EC";
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
