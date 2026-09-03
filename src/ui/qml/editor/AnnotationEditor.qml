import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Rectangle {
    id: editorRoot

    property string imagePath: ""
    property var colors: null

    signal closed()
    signal saved(string newPath)

    anchors.fill: parent
    color: (colors && colors.window) ? colors.window : "#0B1120"
    visible: opacity > 0.0
    opacity: 0.0
    z: 1000

    Behavior on opacity { NumberAnimation { duration: 150 } }

    function open(filePath) {
        imagePath = filePath
        canvas.imageSource = filePath
        opacity = 1.0
    }

    function close() {
        opacity = 0.0
        closed()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // Top Toolbar
        EditorToolbar {
            id: toolbar
            Layout.fillWidth: true
            colors: editorRoot.colors
            canUndo: canvas.undoStack.length > 0
            canRedo: canvas.redoStack.length > 0

            onToolSelected: function(tool) {
                canvas.currentTool = tool
            }
            onColorSelected: function(col) {
                canvas.strokeColor = col
            }
            onUndoRequested: canvas.undo()
            onRedoRequested: canvas.redo()
            onClearRequested: canvas.clear()
            onCloseRequested: editorRoot.close()
            onSaveRequested: {
                // Keep the original untouched and flatten annotations at its
                // native pixel resolution in a sibling PNG export.
                var exportResult = libraryManager.exportAnnotatedImage(
                            editorRoot.imagePath,
                            canvas.annotationDataUrl(),
                            canvas.imageViewportX,
                            canvas.imageViewportY,
                            canvas.imageViewportWidth,
                            canvas.imageViewportHeight)
                if (exportResult.success) {
                    editorRoot.saved(exportResult.path)
                    editorRoot.close()
                }
            }
        }

        // Canvas container
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: (editorRoot.colors && editorRoot.colors.shellAlt) ? editorRoot.colors.shellAlt : "#0F172A"
            border.color: (editorRoot.colors && editorRoot.colors.border) ? editorRoot.colors.border : "#334155"
            border.width: 1
            radius: 8
            clip: true

            EditorCanvas {
                id: canvas
                anchors.fill: parent
                currentTool: toolbar.currentTool
                strokeColor: toolbar.selectedColor
            }
        }
    }
}
