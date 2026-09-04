import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: toolbar

    property int currentTool: 0
    property color selectedColor: "#EF4444"
    property real strokeWidth: 3.0
    property bool canUndo: false
    property bool canRedo: false
    property var colors: null

    signal toolSelected(int tool)
    signal colorSelected(color col)
    signal strokeWidthSelected(real width)
    signal undoRequested()
    signal redoRequested()
    signal clearRequested()
    signal saveRequested()
    signal closeRequested()

    height: 48
    color: (colors && colors.card) ? colors.card : "#FFFFFF"
    border.color: (colors && colors.border) ? colors.border : "#E6E8EC"
    border.width: 1
    radius: 10

    readonly property var tools: [
        { id: 0, label: qsTr("Select"), short: "S" },
        { id: 1, label: qsTr("Crop"), short: "C" },
        { id: 2, label: qsTr("Pen"), short: "P" },
        { id: 3, label: qsTr("Highlight"), short: "H" },
        { id: 4, label: qsTr("Line"), short: "L" },
        { id: 5, label: qsTr("Arrow"), short: "A" },
        { id: 6, label: qsTr("Rectangle"), short: "R" },
        { id: 7, label: qsTr("Ellipse"), short: "O" },
        { id: 8, label: qsTr("Text"), short: "T" },
        { id: 9, label: qsTr("Blur"), short: "B" },
        { id: 10, label: qsTr("Step"), short: "1" }
    ]

    readonly property var colorPalette: [
        "#EF4444", "#F59E0B", "#10B981", "#3B82F6",
        "#8B5CF6", "#EC4899", "#111827", "#FFFFFF"
    ]

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10; anchors.rightMargin: 10
        spacing: 4

        Repeater {
            model: toolbar.tools
            delegate: Button {
                text: modelData.short
                checkable: true
                checked: toolbar.currentTool === modelData.id
                onClicked: { toolbar.currentTool = modelData.id; toolbar.toolSelected(modelData.id) }
                contentItem: Text {
                    text: parent.text
                    color: parent.checked ? "#FFFFFF" : ((toolbar.colors && toolbar.colors.textSoft) ? toolbar.colors.textSoft : "#6B7280")
                    font.pixelSize: 12; font.bold: true
                    horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    implicitWidth: 30; implicitHeight: 30; radius: 7
                    color: parent.checked ? ((toolbar.colors && toolbar.colors.accent) ? toolbar.colors.accent : "#2563EB")
                                          : (parent.hovered ? ((toolbar.colors && toolbar.colors.cardHover) ? toolbar.colors.cardHover : "#F2F4F7") : "transparent")
                }
                ToolTip.visible: hovered; ToolTip.text: modelData.label; ToolTip.delay: 300
            }
        }

        Rectangle { width: 1; height: 22; color: (toolbar.colors && toolbar.colors.border) ? toolbar.colors.border : "#E6E8EC" }

        RowLayout {
            spacing: 5
            Repeater {
                model: toolbar.colorPalette
                delegate: Rectangle {
                    width: 18; height: 18; radius: 9
                    color: modelData
                    border.color: toolbar.selectedColor == modelData ? ((toolbar.colors && toolbar.colors.text) ? toolbar.colors.text : "#111827") : ((toolbar.colors && toolbar.colors.border) ? toolbar.colors.border : "#E6E8EC")
                    border.width: toolbar.selectedColor == modelData ? 2 : 1
                    MouseArea { anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: { toolbar.selectedColor = modelData; toolbar.colorSelected(modelData) } }
                }
            }
        }

        Rectangle { width: 1; height: 22; color: (toolbar.colors && toolbar.colors.border) ? toolbar.colors.border : "#E6E8EC" }

        Button {
            text: "↶"; enabled: toolbar.canUndo; onClicked: toolbar.undoRequested()
            contentItem: Text { text: parent.text; color: parent.enabled ? ((toolbar.colors && toolbar.colors.text) ? toolbar.colors.text : "#111827") : "#9CA3AF"; font.pixelSize: 15; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            background: Rectangle { implicitWidth: 30; implicitHeight: 30; radius: 7; color: parent.hovered && parent.enabled ? ((toolbar.colors && toolbar.colors.cardHover) ? toolbar.colors.cardHover : "#F2F4F7") : "transparent" }
            ToolTip.visible: hovered; ToolTip.text: qsTr("Undo"); ToolTip.delay: 300
        }
        Button {
            text: "↷"; enabled: toolbar.canRedo; onClicked: toolbar.redoRequested()
            contentItem: Text { text: parent.text; color: parent.enabled ? ((toolbar.colors && toolbar.colors.text) ? toolbar.colors.text : "#111827") : "#9CA3AF"; font.pixelSize: 15; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            background: Rectangle { implicitWidth: 30; implicitHeight: 30; radius: 7; color: parent.hovered && parent.enabled ? ((toolbar.colors && toolbar.colors.cardHover) ? toolbar.colors.cardHover : "#F2F4F7") : "transparent" }
            ToolTip.visible: hovered; ToolTip.text: qsTr("Redo"); ToolTip.delay: 300
        }
        Button {
            text: qsTr("Clear"); onClicked: toolbar.clearRequested()
            contentItem: Text { text: parent.text; color: (toolbar.colors && toolbar.colors.textSoft) ? toolbar.colors.textSoft : "#6B7280"; font.pixelSize: 12; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            background: Rectangle { implicitWidth: 52; implicitHeight: 30; radius: 7; color: parent.hovered ? ((toolbar.colors && toolbar.colors.cardHover) ? toolbar.colors.cardHover : "#F2F4F7") : "transparent" }
        }

        Item { Layout.fillWidth: true }

        Button {
            text: qsTr("Save")
            onClicked: toolbar.saveRequested()
            contentItem: Text { text: parent.text; color: "#FFFFFF"; font.bold: true; font.pixelSize: 12; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            background: Rectangle { implicitWidth: 76; implicitHeight: 32; radius: 8; color: parent.hovered ? "#1D4ED8" : "#2563EB" }
        }
        Button {
            text: "✕"; onClicked: toolbar.closeRequested()
            contentItem: Text { text: parent.text; color: (toolbar.colors && toolbar.colors.textSoft) ? toolbar.colors.textSoft : "#6B7280"; font.pixelSize: 13; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            background: Rectangle { implicitWidth: 32; implicitHeight: 32; radius: 8; color: parent.hovered ? ((toolbar.colors && toolbar.colors.cardHover) ? toolbar.colors.cardHover : "#F2F4F7") : "transparent" }
        }
    }
}
