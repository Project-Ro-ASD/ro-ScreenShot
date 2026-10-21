pragma Singleton
import QtQuick
QtObject {
    readonly property real factor: typeof UiPreferencesManager !== "undefined" && UiPreferencesManager !== null ? UiPreferencesManager.animationDurationFactor : 1
}
