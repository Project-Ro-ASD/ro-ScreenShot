import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

Rectangle {
    id: toolbar

    property int currentTool: 0 // 0: Select, 1: Crop, 2: Pen, 3: Highlighter, 4: Line, 5: Arrow, 6: Rect, 7: Ellipse, 8: Text, 9: Blur, 10: Step
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

    height: 52
    color: (colors && colors.shell) ? colors.shell : "#131D31"
    border.color: (colors && colors.border) ? colors.border : "#334155"
    border.width: 1
    radius: 8

    readonly property var tools: [
        { id: 0, label: qsTr("Seç"), icon: "↖" },
        { id: 1, label: qsTr("Kırp"), icon: "✂" },
        { id: 2, label: qsTr("Kalem"), icon: "✏" },
        { id: 3, label: qsTr("Vurgu"), icon: "🖊" },
        { id: 4, label: qsTr("Çizgi"), icon: "╱" },
        { id: 5, label: qsTr("Ok"), icon: "↗" },
        { id: 6, label: qsTr("Kutu"), icon: "▢" },
        { id: 7, label: qsTr("Daire"), icon: "◯" },
        { id: 8, label: qsTr("Metin"), icon: "T" },
        { id: 9, label: qsTr("Bulanık"), icon: "░" },
        { id: 10, label: qsTr("Numara"), icon: "①" }
    ]

    readonly property var palette: [
        "#EF4444", // Red
        "#F59E0B", // Amber
        "#10B981", // Emerald
        "#3B82F6", // Blue
        "#8B5CF6", // Purple
        "#EC4899", // Pink
        "#FFFFFF", // White
        "#000000"  // Black
    ]

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 6

        // Tools Repeater
        Repeater {
            model: toolbar.tools
            delegate: Button {
                text: modelData.icon
                checkable: true
                checked: toolbar.currentTool === modelData.id
                onClicked: {
                    toolbar.currentTool = modelData.id
                    toolbar.toolSelected(modelData.id)
                }
                contentItem: Text {
                    text: parent.text
                    color: parent.checked ? "#FFFFFF" : ((toolbar.colors && toolbar.colors.textSoft) ? toolbar.colors.textSoft : "#94A3B8")
                    font.pixelSize: 14
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    implicitWidth: 34
                    implicitHeight: 34
                    radius: 6
                    color: parent.checked ? ((toolbar.colors && toolbar.colors.accentA) ? toolbar.colors.accentA : "#2563EB")
                                          : (parent.hovered ? ((toolbar.colors && toolbar.colors.cardStrong) ? toolbar.colors.cardStrong : "#1E293B") : "transparent")
                    border.color: parent.checked ? "transparent" : (parent.hovered ? ((toolbar.colors && toolbar.colors.border) ? toolbar.colors.border : "#334155") : "transparent")
                }
                ToolTip.visible: hovered
                ToolTip.text: modelData.label
                ToolTip.delay: 300
            }
        }

        // Separator
        Rectangle {
            width: 1
            height: 24
            color: (toolbar.colors && toolbar.colors.border) ? toolbar.colors.border : "#334155"
        }

        // Color Palette
        RowLayout {
            spacing: 4
            Repeater {
                model: toolbar.palette
                delegate: Rectangle {
                    width: 20
                    height: 20
                    radius: 10
                    color: modelData
                    border.color: toolbar.selectedColor === modelData ? "#FFFFFF" : "#475569"
                    border.width: toolbar.selectedColor === modelData ? 2 : 1

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            toolbar.selectedColor = modelData
                            toolbar.colorSelected(modelData)
                        }
                    }
                }
            }
        }

        // Separator
        Rectangle {
            width: 1
            height: 24
            color: (toolbar.colors && toolbar.colors.border) ? toolbar.colors.border : "#334155"
        }

        // Undo / Redo
        Button {
            text: "↶"
            enabled: toolbar.canUndo
            onClicked: toolbar.undoRequested()
            contentItem: Text {
                text: parent.text
                color: parent.enabled ? "#F8FAFC" : "#64748B"
                font.pixelSize: 16
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                implicitWidth: 32
                implicitHeight: 32
                radius: 6
                color: parent.hovered && parent.enabled ? "#1E293B" : "transparent"
            }
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Geri Al (Undo)")
            ToolTip.delay: 300
        }

        Button {
            text: "↷"
            enabled: toolbar.canRedo
            onClicked: toolbar.redoRequested()
            contentItem: Text {
                text: parent.text
                color: parent.enabled ? "#F8FAFC" : "#64748B"
                font.pixelSize: 16
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                implicitWidth: 32
                implicitHeight: 32
                radius: 6
                color: parent.hovered && parent.enabled ? "#1E293B" : "transparent"
            }
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Yinele (Redo)")
            ToolTip.delay: 300
        }

        Item { Layout.fillWidth: true }

        // Save / Export
        Button {
            text: qsTr("Kaydet")
            onClicked: toolbar.saveRequested()
            contentItem: Text {
                text: parent.text
                color: "#FFFFFF"
                font.bold: true
                font.pixelSize: 12
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                implicitWidth: 72
                implicitHeight: 32
                radius: 6
                color: parent.hovered ? "#16A34A" : "#10B981"
            }
        }

        // Close
        Button {
            text: "✕"
            onClicked: toolbar.closeRequested()
            contentItem: Text {
                text: parent.text
                color: (toolbar.colors && toolbar.colors.textSoft) ? toolbar.colors.textSoft : "#94A3B8"
                font.pixelSize: 13
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                implicitWidth: 32
                implicitHeight: 32
                radius: 6
                color: parent.hovered ? "#1E293B" : "transparent"
            }
        }
    }
}
