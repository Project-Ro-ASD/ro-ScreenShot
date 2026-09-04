import QtQuick
import QtQuick.Controls

// Single premium button. variant: "primary" | "secondary" | "ghost" | "danger" | "dangerGhost"
Button {
    id: control
    property string variant: "secondary"
    property var colors: null
    property int radiusValue: 8

    implicitHeight: 34
    hoverEnabled: true
    focusPolicy: Qt.StrongFocus

    readonly property color _bg: {
        if (!colors) return variant === "primary" ? "#2563EB" : "#F2F4F7"
        if (variant === "primary") return hovered ? colors.accentHover : colors.accent
        if (variant === "danger") return hovered ? "#B91C1C" : colors.danger
        if (variant === "ghost" || variant === "dangerGhost") return hovered ? colors.cardHover : "transparent"
        return hovered ? colors.cardHover : colors.card
    }
    readonly property color _fg: {
        if (!colors) return "#111827"
        if (variant === "primary" || variant === "danger") return "#FFFFFF"
        if (variant === "dangerGhost") return colors.danger
        return colors.text
    }
    readonly property color _bd: {
        if (!colors) return "#E6E8EC"
        if (variant === "primary" || variant === "danger") return "transparent"
        if (variant === "dangerGhost") return colors.danger
        return colors.border
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
        border.color: control._bd
        border.width: (control.variant === "secondary" || control.variant === "ghost" || control.variant === "dangerGhost") ? 1 : 0
    }
}
