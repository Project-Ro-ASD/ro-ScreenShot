pragma Singleton
import QtQuick
import "Palettes.js" as Palettes

QtObject {
    readonly property bool darkMode: typeof UiPreferencesManager !== "undefined" && UiPreferencesManager !== null && UiPreferencesManager.themeMode === "dark"
    readonly property var activePalette: darkMode ? Palettes.dark : Palettes.light

    // Ro system application tokens (Ro-Theme-Docs 02-colors.md)
    readonly property color canvas: activePalette.bg
    readonly property color sidebar: activePalette.surface
    readonly property color shell: activePalette.surfaceAlt
    readonly property color shellAlt: activePalette.surface
    readonly property color card: activePalette.surface
    readonly property color cardStrong: activePalette.surfaceAlt
    readonly property color cardHover: activePalette.surfaceAlt
    readonly property color codeBg: darkMode ? activePalette.bg : activePalette.surfaceAlt
    readonly property color border: activePalette.border
    readonly property color controlBorder: activePalette.controlBorder
    readonly property color text: activePalette.text
    readonly property color textMuted: activePalette.textSecondary
    readonly property color textSoft: activePalette.textSecondary
    readonly property color placeholder: activePalette.disabled
    readonly property color muted: activePalette.disabled
    readonly property color accent: activePalette.accent
    readonly property color accentA: activePalette.accent
    readonly property color accentB: activePalette.accent
    readonly property color accentC: activePalette.accentSoft
    readonly property color accentHover: activePalette.accentHover
    readonly property color accentSoft: activePalette.accentSoft
    readonly property color accentMuted: darkMode ? activePalette.border : activePalette.selection.inactive
    readonly property color selected: activePalette.accentSoft
    readonly property color success: activePalette.positive
    readonly property color warning: activePalette.neutral
    readonly property color danger: activePalette.negative
    readonly property color window: canvas
    readonly property color selectionText: activePalette.selectionText
    readonly property color dangerSoft: activePalette.selection.negative
    readonly property color dangerText: activePalette.bg
    readonly property color scrim: "black"
    readonly property color guide: "white"
    function alpha(value, opacity) { return Qt.rgba(value.r, value.g, value.b, opacity); }
}
