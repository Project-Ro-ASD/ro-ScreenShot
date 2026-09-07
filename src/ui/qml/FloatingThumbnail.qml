import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: toastWindow
    width: 360
    height: 124
    flags: Qt.Window | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.SubWindow
    color: "transparent"

    property string imagePath: ""
    property string fileName: ""
    property bool hasFile: imagePath.length > 0

    x: Screen.virtualX + Screen.desktopAvailableWidth - width - 24
    y: Screen.virtualY + Screen.desktopAvailableHeight - height - 40

    Shortcut {
        sequence: "Escape"
        onActivated: toastWindow.dismiss()
    }
    Timer {
        id: hideTimer
        interval: 5200
        repeat: false
        onTriggered: toastWindow.dismiss()
    }

    function showToast(path, name) {
        imagePath = path || "";
        fileName = name || qsTr("Screenshot");
        hasFile = imagePath.length > 0;
        visible = true;
        hideTimer.restart();
    }
    function dismiss() {
        hideTimer.stop();
        visible = false;
    }

    Rectangle {
        anchors.fill: parent
        radius: 18
        color: "#1B1F2A"
        border.color: "#41485A"
        border.width: 1

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: hideTimer.stop()
            onExited: hideTimer.restart()
        }

        RowLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 12
            Rectangle {
                Layout.preferredWidth: 100
                Layout.preferredHeight: 100
                radius: 12
                color: "#292E3A"
                border.color: "#454D60"
                border.width: 1
                clip: true
                Image {
                    anchors.fill: parent
                    source: toastWindow.imagePath ? "file://" + toastWindow.imagePath : ""
                    fillMode: Image.PreserveAspectCrop
                    asynchronous: true
                }
                Rectangle {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.margins: 7
                    width: 8
                    height: 8
                    radius: 4
                    color: "#20B58A"
                }
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 5
                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: qsTr("Capture complete")
                        color: "#F5F6FA"
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                        Layout.fillWidth: true
                    }
                    Button {
                        text: "×"
                        implicitWidth: 28
                        implicitHeight: 28
                        onClicked: toastWindow.dismiss()
                        contentItem: Text {
                            text: parent.text
                            color: "#9EA7B9"
                            font.pixelSize: 20
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            radius: 8
                            color: parent.hovered ? "#303746" : "transparent"
                        }
                    }
                }
                Text {
                    text: toastWindow.fileName
                    color: "#AAB2C2"
                    font.pixelSize: 11
                    elide: Text.ElideMiddle
                    Layout.fillWidth: true
                }
                Item {
                    Layout.fillHeight: true
                }
                RowLayout {
                    spacing: 7
                    Button {
                        text: qsTr("Copy")
                        onClicked: {
                            captureEngine.copyImageToClipboard(toastWindow.imagePath);
                            toastWindow.dismiss();
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "#FFFFFF"
                            font.pixelSize: 11
                            font.weight: Font.DemiBold
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            implicitWidth: 68
                            implicitHeight: 30
                            radius: 9
                            color: parent.hovered ? "#474BCE" : "#5A5FE8"
                        }
                    }
                    Button {
                        visible: toastWindow.hasFile
                        text: qsTr("Open folder")
                        onClicked: {
                            libraryManager.openInFolder(0);
                            toastWindow.dismiss();
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "#D9DDEA"
                            font.pixelSize: 11
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            implicitWidth: 92
                            implicitHeight: 30
                            radius: 9
                            color: parent.hovered ? "#303746" : "#262B36"
                            border.color: "#454D60"
                            border.width: 1
                        }
                    }
                }
            }
        }
    }
}
