import QtQuick
import QtQuick.Controls

Dialog {
    id: root

    property var colors: null
    property int surfaceRadius: 14
    readonly property color shellColor: colors && colors.shellAlt ? colors.shellAlt : "#1F1B2B"
    readonly property color elevatedColor: colors && colors.cardStrong ? colors.cardStrong : "#342D4A"
    readonly property color borderColor: colors && colors.border ? colors.border : "#4D436B"
    readonly property color textColor: colors && colors.text ? colors.text : "#F8FAFC"
    readonly property color mutedTextColor: colors && colors.textMuted ? colors.textMuted : "#CBD5E1"
    readonly property color accentColor: colors && colors.accent ? colors.accent : "#818CF8"

    modal: true
    focus: true
    padding: 0
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    RoMotion { id: motion }

    enter: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: motion.standard; easing.type: Easing.OutCubic }
            NumberAnimation { property: "scale"; from: motion.dialogHiddenScale; to: 1.0; duration: motion.standard; easing.type: Easing.OutBack }
        }
    }

    exit: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: motion.fast; easing.type: Easing.InCubic }
            NumberAnimation { property: "scale"; from: 1.0; to: motion.dialogHiddenScale; duration: motion.fast; easing.type: Easing.InCubic }
        }
    }

    background: Rectangle {
        radius: root.surfaceRadius
        color: root.colors && root.colors.card ? root.colors.card : "#29233B"
        border.color: root.borderColor
        border.width: 1
    }

    Overlay.modal: Rectangle {
        color: "#000000"
        opacity: 0.46
    }
}
