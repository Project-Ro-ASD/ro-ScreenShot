import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: sniperWindow
    flags: Qt.Window | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.MaximizeUsingFullscreenGeometryHint
    visibility: Window.FullScreen
    color: "transparent"

    property real startX: 0
    property real startY: 0
    property real currentX: 0
    property real currentY: 0
    property bool isSelecting: false
    property bool hasSelection: false
    property int sourceFrameWidth: 0
    property int sourceFrameHeight: 0
    property bool lockSquare: false
    property bool lock16to9: false
    property bool showRulers: true

    property real rawSelW: Math.abs(currentX - startX)
    property real rawSelH: Math.abs(currentY - startY)
    property real selW: lockSquare ? Math.max(rawSelW, rawSelH) : rawSelW
    property real selH: lockSquare ? Math.max(rawSelW, rawSelH) : (lock16to9 ? (rawSelW * 9.0 / 16.0) : rawSelH)
    property real selX: currentX >= startX ? startX : startX - selW
    property real selY: currentY >= startY ? startY : startY - selH

    readonly property real sourceScaleX: sourceFrameWidth > 0 && width > 0 ? sourceFrameWidth / width : 1
    readonly property real sourceScaleY: sourceFrameHeight > 0 && height > 0 ? sourceFrameHeight / height : 1
    readonly property string sampledColor: captureEngine.colorAt(Math.round(mouseArea.mouseX * sourceScaleX), Math.round(mouseArea.mouseY * sourceScaleY))

    Item {
        id: keyboardController
        focus: true
        Component.onCompleted: forceActiveFocus()
        Keys.onEscapePressed: captureEngine.cancelCapture()
        Keys.onReturnPressed: confirmCapture("")
        Keys.onEnterPressed: confirmCapture("")
        Keys.onPressed: event => {
            if (event.key === Qt.Key_Shift)
                lockSquare = true;
            else if (event.key === Qt.Key_Control)
                lock16to9 = true;
            else if (event.key === Qt.Key_R) {
                showRulers = !showRulers;
                event.accepted = true;
            } else if (event.key === Qt.Key_C && !sniperWindow.hasSelection) {
                captureEngine.copyColorAt(Math.round(mouseArea.mouseX * sniperWindow.sourceScaleX), Math.round(mouseArea.mouseY * sniperWindow.sourceScaleY));
                event.accepted = true;
                return;
            }
            if (!sniperWindow.hasSelection)
                return;
            const resize = (event.modifiers & Qt.ShiftModifier) !== 0;
            if (event.key === Qt.Key_Left) {
                sniperWindow.adjustSelection(-1, 0, resize);
                event.accepted = true;
            } else if (event.key === Qt.Key_Right) {
                sniperWindow.adjustSelection(1, 0, resize);
                event.accepted = true;
            } else if (event.key === Qt.Key_Up) {
                sniperWindow.adjustSelection(0, -1, resize);
                event.accepted = true;
            } else if (event.key === Qt.Key_Down) {
                sniperWindow.adjustSelection(0, 1, resize);
                event.accepted = true;
            }
        }
        Keys.onReleased: event => {
            if (event.key === Qt.Key_Shift)
                lockSquare = false;
            else if (event.key === Qt.Key_Control)
                lock16to9 = false;
        }
    }

    function adjustSelection(dx, dy, resize) {
        if (resize) {
            currentX = Math.max(0, Math.min(width, currentX + dx));
            currentY = Math.max(0, Math.min(height, currentY + dy));
            return;
        }
        const nextX = Math.max(0, Math.min(width - selW, selX + dx));
        const nextY = Math.max(0, Math.min(height - selH, selY + dy));
        startX = nextX;
        startY = nextY;
        currentX = nextX + selW;
        currentY = nextY + selH;
    }

    function confirmCapture(action) {
        if (hasSelection && selW > 2 && selH > 2) {
            captureEngine.processRegionSelected(Math.round(selX * sourceScaleX), Math.round(selY * sourceScaleY), Math.round(selW * sourceScaleX), Math.round(selH * sourceScaleY), action || "");
        } else {
            captureEngine.cancelCapture();
        }
    }

    // The window instance is reused between captures. A new frame must always
    // start as a fresh selection session rather than inheriting old handles.
    function prepareForNewFrame() {
        startX = 0;
        startY = 0;
        currentX = 0;
        currentY = 0;
        isSelecting = false;
        hasSelection = false;
        keyboardController.forceActiveFocus();
    }

    Image {
        id: frozenImage
        anchors.fill: parent
        source: "file://" + captureEngine.frozenFramePath
        fillMode: Image.Stretch
        cache: false
    }

    Canvas {
        id: rulerCanvas
        anchors.fill: parent
        visible: sniperWindow.showRulers && !sniperWindow.hasSelection
        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
            ctx.strokeStyle = "rgba(255,255,255,0.25)";
            ctx.fillStyle = "rgba(255,255,255,0.55)";
            ctx.font = "9px monospace";
            for (var x = 0; x < width; x += 50) {
                ctx.beginPath();
                ctx.moveTo(x, 0);
                ctx.lineTo(x, (x % 100 === 0) ? 12 : 7);
                ctx.stroke();
                if (x % 100 === 0 && x > 0)
                    ctx.fillText(x.toString(), x + 3, 11);
            }
            for (var y = 0; y < height; y += 50) {
                ctx.beginPath();
                ctx.moveTo(0, y);
                ctx.lineTo((y % 100 === 0) ? 12 : 7, y);
                ctx.stroke();
                if (y % 100 === 0 && y > 0)
                    ctx.fillText(y.toString(), 3, y + 10);
            }
        }
    }

    Item {
        anchors.fill: parent
        visible: sniperWindow.hasSelection
        Rectangle {
            x: 0
            y: 0
            width: parent.width
            height: sniperWindow.selY
            color: "#99000000"
        }
        Rectangle {
            x: 0
            y: sniperWindow.selY + sniperWindow.selH
            width: parent.width
            height: parent.height - (sniperWindow.selY + sniperWindow.selH)
            color: "#99000000"
        }
        Rectangle {
            x: 0
            y: sniperWindow.selY
            width: sniperWindow.selX
            height: sniperWindow.selH
            color: "#99000000"
        }
        Rectangle {
            x: sniperWindow.selX + sniperWindow.selW
            y: sniperWindow.selY
            width: parent.width - (sniperWindow.selX + sniperWindow.selW)
            height: sniperWindow.selH
            color: "#99000000"
        }
    }
    Rectangle {
        anchors.fill: parent
        color: "#40000000"
        visible: !sniperWindow.hasSelection
    }

    Item {
        anchors.fill: parent
        visible: !sniperWindow.hasSelection
        Rectangle {
            x: 0
            y: mouseArea.mouseY
            width: parent.width
            height: 1
            color: "#80FFFFFF"
        }
        Rectangle {
            x: mouseArea.mouseX
            y: 0
            width: 1
            height: parent.height
            color: "#80FFFFFF"
        }
    }

    // Selection frame — high-contrast instrument edge and indigo handles.
    Rectangle {
        id: selectionBox
        z: 20
        visible: sniperWindow.hasSelection
        x: sniperWindow.selX
        y: sniperWindow.selY
        width: sniperWindow.selW
        height: sniperWindow.selH
        color: "transparent"
        border.color: "#FFFFFF"
        border.width: 1.5

        Rectangle {
            anchors.fill: parent
            color: "transparent"
            border.color: lockSquare ? "#20B58A" : (lock16to9 ? "#D79A32" : "#777CFF")
            border.width: 1
            anchors.margins: -1
        }

        Repeater {
            model: 4
            Rectangle {
                width: 9
                height: 9
                radius: 3
                color: "#FFFFFF"
                border.color: "#777CFF"
                border.width: 1.5
                x: (index % 2 === 0 ? -4 : parent.width - 4)
                y: (index < 2 ? -4 : parent.height - 4)
            }
        }

        Rectangle {
            x: 0
            y: parent.y > 34 ? -30 : 6
            height: 24
            width: dimText.implicitWidth + 16
            color: "#1B1F2A"
            radius: 7
            border.color: "#454D60"
            border.width: 1
            Text {
                id: dimText
                anchors.centerIn: parent
                text: "%1 × %2".arg(Math.round(sniperWindow.selW * sniperWindow.sourceScaleX)).arg(Math.round(sniperWindow.selH * sniperWindow.sourceScaleY))
                color: "#F9FAFB"
                font.pixelSize: 11
                font.bold: true
                font.family: "monospace"
            }
        }

        Rectangle {
            id: miniToolbar
            visible: !sniperWindow.isSelecting && sniperWindow.hasSelection && sniperWindow.selW > 60
            z: 30
            x: Math.max(0, Math.min(parent.width - width, parent.width - width))
            y: parent.height + 10 < (sniperWindow.height - sniperWindow.selY - 52) ? parent.height + 10 : parent.height - 44
            height: 58
            width: toolRow.implicitWidth + 20
            color: "#F0161922"
            radius: 14
            border.color: "#5B647A"
            border.width: 1
            RowLayout {
                id: toolRow
                anchors.centerIn: parent
                spacing: 7
                ColumnLayout {
                    spacing: 1
                    Layout.leftMargin: 4
                    Text { text: qsTr("Selection"); color: "#F5F6FA"; font.pixelSize: 11; font.weight: Font.DemiBold }
                    Text { text: dimText.text; color: "#9EA7B9"; font.pixelSize: 10; font.family: "monospace" }
                }
                Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 30; color: "#454D60" }
                Button {
                    id: copyAction
                    text: qsTr("Copy")
                    implicitWidth: 86
                    implicitHeight: 38
                    focusPolicy: Qt.StrongFocus
                    ToolTip.visible: hovered; ToolTip.text: qsTr("Copy image to clipboard"); ToolTip.delay: 350
                    onClicked: sniperWindow.confirmCapture("copy")
                    contentItem: RowLayout {
                        spacing: 6
                        Text { text: "▣"; color: "#CDD1FF"; font.pixelSize: 14 }
                        Text { text: copyAction.text; color: "#F5F6FA"; font.pixelSize: 12; font.weight: Font.DemiBold }
                    }
                    background: Rectangle {
                        radius: 10
                        color: copyAction.hovered ? "#303746" : "#262B36"
                        border.color: copyAction.activeFocus ? "#8C91FF" : "#4B5366"
                        border.width: copyAction.activeFocus ? 2 : 1
                    }
                }
                Button {
                    id: saveAction
                    text: qsTr("Save")
                    implicitWidth: 88
                    implicitHeight: 38
                    focusPolicy: Qt.StrongFocus
                    ToolTip.visible: hovered; ToolTip.text: qsTr("Save image to library"); ToolTip.delay: 350
                    onClicked: sniperWindow.confirmCapture("save")
                    contentItem: RowLayout {
                        spacing: 6
                        Text { text: "↓"; color: "#FFFFFF"; font.pixelSize: 15; font.weight: Font.DemiBold }
                        Text { text: saveAction.text; color: "#FFFFFF"; font.pixelSize: 12; font.weight: Font.DemiBold }
                    }
                    background: Rectangle {
                        radius: 10
                        color: saveAction.hovered ? "#474BCE" : "#5A5FE8"
                        border.color: saveAction.activeFocus ? "#C9CBFF" : "transparent"
                        border.width: saveAction.activeFocus ? 2 : 0
                    }
                }
                Button {
                    id: gifAction
                    text: "GIF"
                    visible: screenRecorderEngine !== null
                    implicitWidth: 68
                    implicitHeight: 38
                    focusPolicy: Qt.StrongFocus
                    ToolTip.visible: hovered; ToolTip.text: qsTr("Record this region as a 5-second GIF"); ToolTip.delay: 350
                    onClicked: {
                        var reg = Qt.rect(Math.round(sniperWindow.selX * sniperWindow.sourceScaleX), Math.round(sniperWindow.selY * sniperWindow.sourceScaleY), Math.round(sniperWindow.selW * sniperWindow.sourceScaleX), Math.round(sniperWindow.selH * sniperWindow.sourceScaleY));
                        screenRecorderEngine.startRegionGif(reg, 5);
                        captureEngine.cancelCapture();
                    }
                    contentItem: RowLayout {
                        spacing: 5
                        Text { text: "◌"; color: "#CDD1FF"; font.pixelSize: 15 }
                        Text { text: gifAction.text; color: "#CDD1FF"; font.pixelSize: 11
                        font.bold: true
                        }
                    }
                    background: Rectangle {
                        radius: 10
                        color: gifAction.hovered ? "#2E3355" : "transparent"
                        border.color: gifAction.activeFocus ? "#8C91FF" : "#4B5366"
                        border.width: gifAction.activeFocus ? 2 : 1
                    }
                }
                Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 30; color: "#454D60" }
                Button {
                    id: cancelAction
                    text: "×"
                    implicitWidth: 34
                    implicitHeight: 38
                    focusPolicy: Qt.StrongFocus
                    ToolTip.visible: hovered; ToolTip.text: qsTr("Cancel selection (Esc)"); ToolTip.delay: 350
                    onClicked: captureEngine.cancelCapture()
                    contentItem: Text {
                        text: cancelAction.text
                        color: "#AAB2C2"
                        font.pixelSize: 20
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        radius: 10
                        color: cancelAction.hovered ? "#39303A" : "transparent"
                        border.color: cancelAction.activeFocus ? "#C4475D" : "transparent"
                        border.width: cancelAction.activeFocus ? 2 : 0
                    }
                }
            }
        }
    }

    // Loupe
    Rectangle {
        z: 30
        id: magnifier
        visible: settingsManager.magnifierEnabled && !sniperWindow.hasSelection
        width: 136
        height: 136
        radius: 68
        color: "#1B1F2A"
        border.color: "#777CFF"
        border.width: 2
        clip: true
        x: (mouseArea.mouseX + 20 + width > sniperWindow.width) ? mouseArea.mouseX - width - 20 : mouseArea.mouseX + 20
        y: (mouseArea.mouseY + 20 + height > sniperWindow.height) ? mouseArea.mouseY - height - 20 : mouseArea.mouseY + 20
        Item {
            anchors.fill: parent
            clip: true
            Image {
                source: frozenImage.source
                width: frozenImage.width * settingsManager.magnifierZoom
                height: frozenImage.height * settingsManager.magnifierZoom
                x: -(mouseArea.mouseX * settingsManager.magnifierZoom - parent.width / 2)
                y: -(mouseArea.mouseY * settingsManager.magnifierZoom - parent.height / 2)
            }
            Rectangle {
                anchors.centerIn: parent
                width: parent.width
                height: 1
                color: "#60FFFFFF"
            }
            Rectangle {
                anchors.centerIn: parent
                width: 1
                height: parent.height
                color: "#60FFFFFF"
            }
            Rectangle {
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottomMargin: 8
                height: 20
                width: Math.min(parent.width - 16, hexLabel.implicitWidth + 14)
                radius: 5
                color: "#E5111827"
                Text {
                    id: hexLabel
                    anchors.centerIn: parent
                    text: sniperWindow.sampledColor
                    color: "#F9FAFB"
                    font.pixelSize: 10
                    font.bold: true
                    font.family: "monospace"
                }
            }
        }
    }

    // Bottom hint pill
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 22
        height: 30
        width: hintRow.implicitWidth + 28
        radius: 15
        color: "#E01B1F2A"
        border.color: "#454D60"
        border.width: 1
        visible: !sniperWindow.hasSelection
        RowLayout {
            id: hintRow
            anchors.centerIn: parent
            spacing: 12
            Text {
                text: qsTr("Drag to select  ·  Enter confirm  ·  C copy color  ·  Esc cancel")
                color: "#D1D5DB"
                font.pixelSize: 11
            }
        }
    }

    Rectangle {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 20
        height: 32
        width: modeRow.implicitWidth + 24
        radius: 16
        color: "#E01B1F2A"
        border.color: "#454D60"
        border.width: 1
        RowLayout {
            id: modeRow
            anchors.centerIn: parent
            spacing: 8
            Rectangle {
                width: 7
                height: 7
                radius: 4
                color: sniperWindow.lockSquare ? "#20B58A" : (sniperWindow.lock16to9 ? "#D79A32" : "#777CFF")
            }
            Text {
                text: sniperWindow.lockSquare ? qsTr("Square selection") : (sniperWindow.lock16to9 ? qsTr("16:9 selection") : qsTr("Precision selection"))
                color: "#F5F6FA"
                font.pixelSize: 11
                font.weight: Font.DemiBold
            }
            Text {
                text: qsTr("Esc to cancel")
                color: "#AAB2C2"
                font.pixelSize: 10
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.CrossCursor
        onPressed: mouse => {
            keyboardController.forceActiveFocus();
            sniperWindow.startX = mouse.x;
            sniperWindow.startY = mouse.y;
            sniperWindow.currentX = mouse.x;
            sniperWindow.currentY = mouse.y;
            sniperWindow.isSelecting = true;
            sniperWindow.hasSelection = true;
        }
        onPositionChanged: mouse => {
            if (sniperWindow.isSelecting) {
                sniperWindow.currentX = mouse.x;
                sniperWindow.currentY = mouse.y;
            }
        }
        onReleased: mouse => {
            sniperWindow.isSelecting = false;
            sniperWindow.currentX = mouse.x;
            sniperWindow.currentY = mouse.y;
            if (sniperWindow.selW < 4 && sniperWindow.selH < 4)
                sniperWindow.hasSelection = false;
        }
        onDoubleClicked: sniperWindow.confirmCapture("")
    }
}
