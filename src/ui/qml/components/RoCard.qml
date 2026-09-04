import QtQuick

// Premium minimal card. Flat, 1px border, 12px radius, no glow.
Rectangle {
    id: root
    property bool hovered: false
    property bool selected: false
    property var colors: null
    property int radiusValue: 12

    radius: radiusValue
    color: {
        if (!colors) return "#FFFFFF"
        if (selected) return colors.accentSoft
        if (hovered) return colors.cardHover
        return colors.card
    }
    border.color: {
        if (!colors) return "#E6E8EC"
        if (selected) return colors.accent
        if (hovered) return colors.accentMuted
        return colors.border
    }
    border.width: (selected || hovered) ? 1.5 : 1
}
