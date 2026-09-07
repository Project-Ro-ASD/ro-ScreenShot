import QtQuick
import QtQuick.Controls

Dialog {
    id: root

    property var colors: null
    property int surfaceRadius: 14

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
        border.color: root.colors && root.colors.border ? root.colors.border : "#4D436B"
        border.width: 1
    }

    Overlay.modal: Rectangle {
        color: "#000000"
        opacity: 0.46
    }
}
