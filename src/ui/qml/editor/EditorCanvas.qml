import QtQuick
import QtQuick.Controls

Item {
    id: canvasArea

    property string imageSource: ""
    property int currentTool: 0
    property color strokeColor: "#EF4444"
    property real strokeWidth: 3.0

    property var annotations: [] // Array of annotation items
    property var currentPath: [] // For active freehand pen / highlighter
    property point startPoint: Qt.point(0, 0)
    property point endPoint: Qt.point(0, 0)
    property bool isDrawing: false
    property int stepCounter: 1

    property var undoStack: []
    property var redoStack: []

    signal changed()

    readonly property real imageViewportX: bgImage.x + (bgImage.width - bgImage.paintedWidth) / 2
    readonly property real imageViewportY: bgImage.y + (bgImage.height - bgImage.paintedHeight) / 2
    readonly property real imageViewportWidth: bgImage.paintedWidth
    readonly property real imageViewportHeight: bgImage.paintedHeight

    function annotationDataUrl() {
        return paintCanvas.toDataURL("image/png")
    }

    function pushCommand(action) {
        undoStack.push(JSON.parse(JSON.stringify(annotations)))
        redoStack = []
        action()
        paintCanvas.requestPaint()
        canvasArea.changed()
    }

    function undo() {
        if (undoStack.length > 0) {
            redoStack.push(JSON.parse(JSON.stringify(annotations)))
            annotations = undoStack.pop()
            paintCanvas.requestPaint()
            canvasArea.changed()
        }
    }

    function redo() {
        if (redoStack.length > 0) {
            undoStack.push(JSON.parse(JSON.stringify(annotations)))
            annotations = redoStack.pop()
            paintCanvas.requestPaint()
            canvasArea.changed()
        }
    }

    function clear() {
        pushCommand(function() {
            annotations = []
            stepCounter = 1
        })
    }

    Image {
        id: bgImage
        anchors.fill: parent
        source: canvasArea.imageSource ? ("file://" + canvasArea.imageSource) : ""
        fillMode: Image.PreserveAspectFit
        asynchronous: true
        onStatusChanged: {
            if (status === Image.Ready) {
                paintCanvas.requestPaint()
            }
        }
    }

    Canvas {
        id: paintCanvas
        anchors.fill: parent

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            // Render existing annotations
            for (var i = 0; i < annotations.length; ++i) {
                var item = annotations[i]
                ctx.strokeStyle = item.color
                ctx.fillStyle = item.fillColor || "transparent"
                ctx.lineWidth = item.width || 3.0
                ctx.lineCap = "round"
                ctx.lineJoin = "round"

                if (item.type === "rect") {
                    ctx.beginPath()
                    ctx.rect(item.x, item.y, item.w, item.h)
                    if (item.filled) ctx.fill()
                    ctx.stroke()
                } else if (item.type === "ellipse") {
                    ctx.beginPath()
                    ctx.ellipse(item.x, item.y, item.w, item.h)
                    if (item.filled) ctx.fill()
                    ctx.stroke()
                } else if (item.type === "line") {
                    ctx.beginPath()
                    ctx.moveTo(item.x1, item.y1)
                    ctx.lineTo(item.x2, item.y2)
                    ctx.stroke()
                } else if (item.type === "arrow") {
                    drawArrow(ctx, item.x1, item.y1, item.x2, item.y2, item.color, item.width)
                } else if (item.type === "path") {
                    if (item.points && item.points.length > 1) {
                        ctx.beginPath()
                        ctx.moveTo(item.points[0].x, item.points[0].y)
                        for (var p = 1; p < item.points.length; ++p) {
                            ctx.lineTo(item.points[p].x, item.points[p].y)
                        }
                        ctx.stroke()
                    }
                } else if (item.type === "text") {
                    ctx.fillStyle = item.color
                    ctx.font = "bold " + (item.fontSize || 16) + "px sans-serif"
                    ctx.fillText(item.text, item.x, item.y)
                } else if (item.type === "blur") {
                    ctx.fillStyle = "rgba(100, 116, 139, 0.65)"
                    ctx.fillRect(item.x, item.y, item.w, item.h)
                    ctx.strokeStyle = "#94A3B8"
                    ctx.lineWidth = 1
                    ctx.strokeRect(item.x, item.y, item.w, item.h)
                } else if (item.type === "step") {
                    ctx.fillStyle = item.color
                    ctx.beginPath()
                    ctx.arc(item.x, item.y, 14, 0, Math.PI * 2)
                    ctx.fill()

                    ctx.fillStyle = "#FFFFFF"
                    ctx.font = "bold 12px sans-serif"
                    ctx.textAlign = "center"
                    ctx.textBaseline = "middle"
                    ctx.fillText("" + item.number, item.x, item.y)
                }
            }

            // Render active drawing preview
            if (canvasArea.isDrawing) {
                ctx.strokeStyle = canvasArea.strokeColor
                ctx.lineWidth = canvasArea.strokeWidth
                ctx.lineCap = "round"
                ctx.lineJoin = "round"

                if (canvasArea.currentTool === 2 || canvasArea.currentTool === 3) { // Pen or Highlighter
                    if (currentPath.length > 1) {
                        ctx.beginPath()
                        ctx.moveTo(currentPath[0].x, currentPath[0].y)
                        for (var k = 1; k < currentPath.length; ++k) {
                            ctx.lineTo(currentPath[k].x, currentPath[k].y)
                        }
                        ctx.stroke()
                    }
                } else if (canvasArea.currentTool === 4) { // Line
                    ctx.beginPath()
                    ctx.moveTo(startPoint.x, startPoint.y)
                    ctx.lineTo(endPoint.x, endPoint.y)
                    ctx.stroke()
                } else if (canvasArea.currentTool === 5) { // Arrow
                    drawArrow(ctx, startPoint.x, startPoint.y, endPoint.x, endPoint.y, canvasArea.strokeColor, canvasArea.strokeWidth)
                } else if (canvasArea.currentTool === 6) { // Rect
                    var rx = Math.min(startPoint.x, endPoint.x)
                    var ry = Math.min(startPoint.y, endPoint.y)
                    var rw = Math.abs(endPoint.x - startPoint.x)
                    var rh = Math.abs(endPoint.y - startPoint.y)
                    ctx.strokeRect(rx, ry, rw, rh)
                } else if (canvasArea.currentTool === 7) { // Ellipse
                    var ex = Math.min(startPoint.x, endPoint.x)
                    var ey = Math.min(startPoint.y, endPoint.y)
                    var ew = Math.abs(endPoint.x - startPoint.x)
                    var eh = Math.abs(endPoint.y - startPoint.y)
                    ctx.beginPath()
                    ctx.ellipse(ex + ew/2, ey + eh/2, ew/2, eh/2)
                    ctx.stroke()
                } else if (canvasArea.currentTool === 9) { // Blur
                    var bx = Math.min(startPoint.x, endPoint.x)
                    var by = Math.min(startPoint.y, endPoint.y)
                    var bw = Math.abs(endPoint.x - startPoint.x)
                    var bh = Math.abs(endPoint.y - startPoint.y)
                    ctx.fillStyle = "rgba(100, 116, 139, 0.45)"
                    ctx.fillRect(bx, by, bw, bh)
                }
            }
        }

        function drawArrow(ctx, x1, y1, x2, y2, color, width) {
            var headLen = Math.max(12, width * 3.5)
            var angle = Math.atan2(y2 - y1, x2 - x1)

            ctx.strokeStyle = color
            ctx.fillStyle = color
            ctx.lineWidth = width
            ctx.beginPath()
            ctx.moveTo(x1, y1)
            ctx.lineTo(x2, y2)
            ctx.stroke()

            ctx.beginPath()
            ctx.moveTo(x2, y2)
            ctx.lineTo(x2 - headLen * Math.cos(angle - Math.PI / 7), y2 - headLen * Math.sin(angle - Math.PI / 7))
            ctx.lineTo(x2 - headLen * Math.cos(angle + Math.PI / 7), y2 - headLen * Math.sin(angle + Math.PI / 7))
            ctx.closePath()
            ctx.fill()
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: {
            if (currentTool === 0) return Qt.ArrowCursor
            if (currentTool === 8) return Qt.IBeamCursor
            return Qt.CrossCursor
        }

        onPressed: function(mouse) {
            canvasArea.startPoint = Qt.point(mouse.x, mouse.y)
            canvasArea.endPoint = Qt.point(mouse.x, mouse.y)
            canvasArea.isDrawing = true

            if (canvasArea.currentTool === 2 || canvasArea.currentTool === 3) {
                canvasArea.currentPath = [Qt.point(mouse.x, mouse.y)]
            } else if (canvasArea.currentTool === 10) { // Step marker
                canvasArea.pushCommand(function() {
                    canvasArea.annotations.push({
                        type: "step",
                        x: mouse.x,
                        y: mouse.y,
                        color: canvasArea.strokeColor.toString(),
                        number: canvasArea.stepCounter++
                    })
                })
                canvasArea.isDrawing = false
            }
            paintCanvas.requestPaint()
        }

        onPositionChanged: function(mouse) {
            if (!canvasArea.isDrawing) return
            canvasArea.endPoint = Qt.point(mouse.x, mouse.y)

            if (canvasArea.currentTool === 2 || canvasArea.currentTool === 3) {
                canvasArea.currentPath.push(Qt.point(mouse.x, mouse.y))
            }
            paintCanvas.requestPaint()
        }

        onReleased: function(mouse) {
            if (!canvasArea.isDrawing) return
            canvasArea.isDrawing = false
            canvasArea.endPoint = Qt.point(mouse.x, mouse.y)

            if (canvasArea.currentTool === 2 || canvasArea.currentTool === 3) { // Pen / Highlighter
                if (canvasArea.currentPath.length > 1) {
                    var pts = JSON.parse(JSON.stringify(canvasArea.currentPath))
                    var isHigh = canvasArea.currentTool === 3
                    canvasArea.pushCommand(function() {
                        canvasArea.annotations.push({
                            type: "path",
                            points: pts,
                            color: isHigh ? "rgba(239, 68, 68, 0.4)" : canvasArea.strokeColor.toString(),
                            width: isHigh ? 16 : canvasArea.strokeWidth
                        })
                    })
                }
            } else if (canvasArea.currentTool === 4) { // Line
                canvasArea.pushCommand(function() {
                    canvasArea.annotations.push({
                        type: "line",
                        x1: canvasArea.startPoint.x,
                        y1: canvasArea.startPoint.y,
                        x2: canvasArea.endPoint.x,
                        y2: canvasArea.endPoint.y,
                        color: canvasArea.strokeColor.toString(),
                        width: canvasArea.strokeWidth
                    })
                })
            } else if (canvasArea.currentTool === 5) { // Arrow
                canvasArea.pushCommand(function() {
                    canvasArea.annotations.push({
                        type: "arrow",
                        x1: canvasArea.startPoint.x,
                        y1: canvasArea.startPoint.y,
                        x2: canvasArea.endPoint.x,
                        y2: canvasArea.endPoint.y,
                        color: canvasArea.strokeColor.toString(),
                        width: canvasArea.strokeWidth
                    })
                })
            } else if (canvasArea.currentTool === 6) { // Rect
                var rx = Math.min(canvasArea.startPoint.x, canvasArea.endPoint.x)
                var ry = Math.min(canvasArea.startPoint.y, canvasArea.endPoint.y)
                var rw = Math.abs(canvasArea.endPoint.x - canvasArea.startPoint.x)
                var rh = Math.abs(canvasArea.endPoint.y - canvasArea.startPoint.y)
                if (rw > 3 && rh > 3) {
                    canvasArea.pushCommand(function() {
                        canvasArea.annotations.push({
                            type: "rect",
                            x: rx,
                            y: ry,
                            w: rw,
                            h: rh,
                            color: canvasArea.strokeColor.toString(),
                            width: canvasArea.strokeWidth
                        })
                    })
                }
            } else if (canvasArea.currentTool === 7) { // Ellipse
                var ex = Math.min(canvasArea.startPoint.x, canvasArea.endPoint.x)
                var ey = Math.min(canvasArea.startPoint.y, canvasArea.endPoint.y)
                var ew = Math.abs(canvasArea.endPoint.x - canvasArea.startPoint.x)
                var eh = Math.abs(canvasArea.endPoint.y - canvasArea.startPoint.y)
                if (ew > 3 && eh > 3) {
                    canvasArea.pushCommand(function() {
                        canvasArea.annotations.push({
                            type: "ellipse",
                            x: ex + ew/2,
                            y: ey + eh/2,
                            w: ew/2,
                            h: eh/2,
                            color: canvasArea.strokeColor.toString(),
                            width: canvasArea.strokeWidth
                        })
                    })
                }
            } else if (canvasArea.currentTool === 9) { // Blur
                var bx = Math.min(canvasArea.startPoint.x, canvasArea.endPoint.x)
                var by = Math.min(canvasArea.startPoint.y, canvasArea.endPoint.y)
                var bw = Math.abs(canvasArea.endPoint.x - canvasArea.startPoint.x)
                var bh = Math.abs(canvasArea.endPoint.y - canvasArea.startPoint.y)
                if (bw > 3 && bh > 3) {
                    canvasArea.pushCommand(function() {
                        canvasArea.annotations.push({
                            type: "blur",
                            x: bx,
                            y: by,
                            w: bw,
                            h: bh
                        })
                    })
                }
            }

            paintCanvas.requestPaint()
        }
    }
}
