import QtQuick

// Shared Ro system-app motion contract. Keep feedback quick, calm, and
// consistent across controls, transient surfaces, and modal interactions.
QtObject {
    readonly property int instant: 90
    readonly property int fast: 120
    readonly property int standard: 180
    readonly property int slow: 240
    readonly property int tooltipDelay: 350
    readonly property int tooltipTimeout: 4000
    readonly property real pressedScale: 0.98
    readonly property real dialogHiddenScale: 0.96
}
