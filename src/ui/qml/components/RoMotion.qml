import QtQuick

// Shared Ro system-app motion contract. Keep feedback quick, calm, and
// consistent across controls, transient surfaces, and modal interactions.
QtObject {
    readonly property int instant: Math.round(90 * RoMotionScale.factor)
    readonly property int fast: Math.round(120 * RoMotionScale.factor)
    readonly property int standard: Math.round(180 * RoMotionScale.factor)
    readonly property int slow: Math.round(240 * RoMotionScale.factor)
    readonly property int tooltipDelay: 350
    readonly property int tooltipTimeout: 4000
    readonly property real pressedScale: 0.98
    readonly property real dialogHiddenScale: 0.96
}
