import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: editorRoot

    property string imagePath: ""
    property var colors: null

    signal closed()
    signal saved(string newPath)

    anchors.fill: parent
    color: (colors && colors.window) ? colors.window : "#F7F8FA"
    visible: opacity > 0.0
    opacity: 0.0
    z: 1000

    Behavior on opacity { NumberAnimation { duration: 150 } }

    function open(filePath) {
        imagePath = filePath
        canvas.imageSource = filePath
        opacity = 1.0
    }
    function close() { opacity = 0.0; closed() }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            Text { text: qsTr("Annotate"); color: (editorRoot.colors && editorRoot.colors.text) ? editorRoot.colors.text : "#111827"; font.pixelSize: 16; font.bold: true; Layout.fillWidth: true }
            Text { text: editorRoot.imagePath.split("/").pop(); color: (editorRoot.colors && editorRoot.colors.placeholder) ? editorRoot.colors.placeholder : "#9CA3AF"; font.pixelSize: 11; elide: Text.ElideMiddle; Layout.preferredWidth: 300; horizontalAlignment: Text.AlignRight }
        }

        EditorToolbar {
            id: toolbar
            Layout.fillWidth: true
            colors: editorRoot.colors
            canUndo: canvas.undoStack.length > 0
            canRedo: canvas.redoStack.length > 0
            onToolSelected: function(tool) { canvas.currentTool = tool }
            onColorSelected: function(col) { canvas.strokeColor = col }
            onUndoRequested: canvas.undo()
            onRedoRequested: canvas.redo()
            onClearRequested: canvas.clear()
            onCloseRequested: editorRoot.close()
            onSaveRequested: {
                var exportResult = libraryManager.exportAnnotatedImage(
                    editorRoot.imagePath, canvas.annotationDataUrl(),
                    canvas.imageViewportX, canvas.imageViewportY,
                    canvas.imageViewportWidth, canvas.imageViewportHeight)
                if (exportResult.success) { editorRoot.saved(exportResult.path); editorRoot.close() }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: (editorRoot.colors && editorRoot.colors.card) ? editorRoot.colors.card : "#FFFFFF"
            border.color: (editorRoot.colors && editorRoot.colors.border) ? editorRoot.colors.border : "#E6E8EC"
            border.width: 1
            radius: 12
            clip: true
            EditorCanvas {
                id: canvas
                anchors.fill: parent
                anchors.margins: 1
                currentTool: toolbar.currentTool
                strokeColor: toolbar.selectedColor
            }
        }
    }
}
