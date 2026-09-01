import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

ApplicationWindow {
    id: root
    visible: true
    width: 1080
    height: 720
    minimumWidth: 880
    minimumHeight: 600
    title: qsTr("Ro-ScreenShot Hub")
    color: "#0B1120"

    property int currentTab: 0 // 0: Quick Capture, 1: Library, 2: Settings

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // ─── Left Sidebar Navigation (no brand header, icon+label only) ─────────
        Rectangle {
            Layout.preferredWidth: 230
            Layout.fillHeight: true
            color: "#131D31"
            border.color: "#1E293B"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 8

                // Nav Menu Items
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    // Nav Button 1: Quick Capture
                    Rectangle {
                        Layout.fillWidth: true
                        height: 42
                        radius: 8
                        color: root.currentTab === 0 ? "#2563EB" : (btn1Mouse.containsMouse ? "#1E293B" : "transparent")

                        MouseArea {
                            id: btn1Mouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.currentTab = 0
                        }

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            spacing: 12

                            Image {
                                source: "assets/icon-capture.svg"
                                sourceSize.width: 20
                                sourceSize.height: 20
                                Layout.preferredWidth: 20
                                Layout.preferredHeight: 20
                            }

                            Text {
                                text: qsTr("Hızlı Çekim")
                                color: root.currentTab === 0 ? "#FFFFFF" : "#94A3B8"
                                font.pixelSize: 13
                                font.bold: root.currentTab === 0
                                Layout.fillWidth: true
                            }
                        }
                    }

                    // Nav Button 2: Library / Gallery
                    Rectangle {
                        Layout.fillWidth: true
                        height: 42
                        radius: 8
                        color: root.currentTab === 1 ? "#2563EB" : (btn2Mouse.containsMouse ? "#1E293B" : "transparent")

                        MouseArea {
                            id: btn2Mouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.currentTab = 1
                        }

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            spacing: 12

                            Image {
                                source: "assets/icon-gallery.svg"
                                sourceSize.width: 20
                                sourceSize.height: 20
                                Layout.preferredWidth: 20
                                Layout.preferredHeight: 20
                            }

                            Text {
                                text: qsTr("Galeri")
                                color: root.currentTab === 1 ? "#FFFFFF" : "#94A3B8"
                                font.pixelSize: 13
                                font.bold: root.currentTab === 1
                                Layout.fillWidth: true
                            }

                            Rectangle {
                                height: 20
                                width: countBadgeText.implicitWidth + 10
                                radius: 10
                                color: root.currentTab === 1 ? "#1D4ED8" : "#1E293B"
                                Text {
                                    id: countBadgeText
                                    anchors.centerIn: parent
                                    text: libraryManager.count
                                    color: root.currentTab === 1 ? "#FFFFFF" : "#94A3B8"
                                    font.pixelSize: 10
                                    font.bold: true
                                }
                            }
                        }
                    }

                    // Nav Button 3: Settings
                    Rectangle {
                        Layout.fillWidth: true
                        height: 42
                        radius: 8
                        color: root.currentTab === 2 ? "#2563EB" : (btn3Mouse.containsMouse ? "#1E293B" : "transparent")

                        MouseArea {
                            id: btn3Mouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.currentTab = 2
                        }

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            spacing: 12

                            Image {
                                source: "assets/icon-settings.svg"
                                sourceSize.width: 20
                                sourceSize.height: 20
                                Layout.preferredWidth: 20
                                Layout.preferredHeight: 20
                            }

                            Text {
                                text: qsTr("Ayarlar")
                                color: root.currentTab === 2 ? "#FFFFFF" : "#94A3B8"
                                font.pixelSize: 13
                                font.bold: root.currentTab === 2
                                Layout.fillWidth: true
                            }
                        }
                    }
                }

                Item { Layout.fillHeight: true }

                // Bottom Sidebar: Language Selector (toggle buttons, icon-free) + Storage Status
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: "#1E293B"
                    }

                    // Language Row: native label toggle buttons (ro-Control style)
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 6

                        Text {
                            text: qsTr("Dil")
                            color: "#94A3B8"
                            font.pixelSize: 11
                            font.bold: true
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 6

                            Repeater {
                                model: languageManager.availableLanguages
                                delegate: Button {
                                    required property var modelData
                                    Layout.fillWidth: true
                                    implicitHeight: 32
                                    text: modelData.nativeLabel
                                    checkable: true
                                    checked: languageManager.currentLanguage === modelData.code
                                    onClicked: languageManager.setCurrentLanguage(modelData.code)
                                    contentItem: Text {
                                        text: parent.text
                                        color: parent.checked ? "#FFFFFF" : "#CBD5E1"
                                        font.pixelSize: 11
                                        font.bold: parent.checked
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    background: Rectangle {
                                        radius: 6
                                        color: parent.checked ? "#2563EB" : (parent.hovered ? "#1E293B" : "#131D31")
                                        border.color: parent.checked ? "#2563EB" : "#334155"
                                        border.width: 1
                                    }
                                }
                            }
                        }
                    }

                    // Storage Badge
                    Rectangle {
                        Layout.fillWidth: true
                        height: 28
                        radius: 6
                        color: "#0F172A"
                        border.color: "#1E293B"

                        RowLayout {
                            anchors.centerIn: parent
                            spacing: 6

                            Image {
                                source: "assets/icon-folder.svg"
                                sourceSize.width: 14
                                sourceSize.height: 14
                                Layout.preferredWidth: 14
                                Layout.preferredHeight: 14
                            }

                            Text {
                                text: libraryManager.totalStorageSize
                                color: "#94A3B8"
                                font.pixelSize: 10
                            }
                        }
                    }
                }
            }
        }

        // ─── Main Content Area ────────────────────────────────────────────────
        StackLayout {
            id: contentStack
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: root.currentTab

            // TAB 0: Quick Capture Hub
            Item {
                id: quickCaptureTab

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 32
                    width: Math.min(parent.width - 64, 860)

                    ColumnLayout {
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 8
                        Text {
                            text: qsTr("Ekran Görüntüsü Yakalama Merkezi")
                            color: "#F8FAFC"
                            font.pixelSize: 24
                            font.bold: true
                            Layout.alignment: Qt.AlignHCenter
                        }
                        Text {
                            text: qsTr("Bir çekim modu seçin veya sistem kısayollarını kullanın.")
                            color: "#94A3B8"
                            font.pixelSize: 14
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }

                    // Grid of 4 Action Cards with Vector Icons
                    GridLayout {
                        columns: 2
                        rowSpacing: 20
                        columnSpacing: 20
                        Layout.fillWidth: true

                        // Card 1: Region
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 140
                            color: regionMouse.containsMouse ? "#1E293B" : "#131D31"
                            border.color: regionMouse.containsMouse ? "#3B82F6" : "#334155"
                            border.width: 1.5
                            radius: 12

                            MouseArea {
                                id: regionMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    captureEngine.requestRegionCapture(0)
                                }
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 20
                                spacing: 16

                                Rectangle {
                                    width: 56
                                    height: 56
                                    radius: 10
                                    color: "#1D4ED8"
                                    Image {
                                        anchors.centerIn: parent
                                        source: "assets/icon-capture.svg"
                                        sourceSize.width: 28
                                        sourceSize.height: 28
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 4
                                    Text {
                                        text: qsTr("Bölge Seçimi (Sniper)")
                                        color: "#F8FAFC"
                                        font.pixelSize: 16
                                        font.bold: true
                                    }
                                    Text {
                                        text: qsTr("Dondurulmuş ekranda serbest dikdörtgen alanı seçin ve kırpın.")
                                        color: "#94A3B8"
                                        font.pixelSize: 12
                                        wrapMode: Text.WordWrap
                                        Layout.fillWidth: true
                                    }
                                    Rectangle {
                                        height: 22
                                        width: 120
                                        radius: 4
                                        color: "#334155"
                                        Text {
                                            anchors.centerIn: parent
                                            text: "Shift + PrtScr"
                                            color: "#E2E8F0"
                                            font.pixelSize: 11
                                        }
                                    }
                                }
                            }
                        }

                        // Card 2: Fullscreen
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 140
                            color: fullMouse.containsMouse ? "#1E293B" : "#131D31"
                            border.color: fullMouse.containsMouse ? "#10B981" : "#334155"
                            border.width: 1.5
                            radius: 12

                            MouseArea {
                                id: fullMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    captureEngine.requestFullscreenCapture(0)
                                }
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 20
                                spacing: 16

                                Rectangle {
                                    width: 56
                                    height: 56
                                    radius: 10
                                    color: "#047857"
                                    Image {
                                        anchors.centerIn: parent
                                        source: "assets/icon-fullscreen.svg"
                                        sourceSize.width: 28
                                        sourceSize.height: 28
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 4
                                    Text {
                                        text: qsTr("Tam Ekran Yakala")
                                        color: "#F8FAFC"
                                        font.pixelSize: 16
                                        font.bold: true
                                    }
                                    Text {
                                        text: qsTr("Tüm monitörlerin görüntüsünü anında yakalayın ve kaydedin.")
                                        color: "#94A3B8"
                                        font.pixelSize: 12
                                        wrapMode: Text.WordWrap
                                        Layout.fillWidth: true
                                    }
                                    Rectangle {
                                        height: 22
                                        width: 80
                                        radius: 4
                                        color: "#334155"
                                        Text {
                                            anchors.centerIn: parent
                                            text: "PrtScr"
                                            color: "#E2E8F0"
                                            font.pixelSize: 11
                                        }
                                    }
                                }
                            }
                        }

                        // Card 3: Window
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 140
                            color: winMouse.containsMouse ? "#1E293B" : "#131D31"
                            border.color: winMouse.containsMouse ? "#8B5CF6" : "#334155"
                            border.width: 1.5
                            radius: 12

                            MouseArea {
                                id: winMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    captureEngine.requestWindowCapture(0)
                                }
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 20
                                spacing: 16

                                Rectangle {
                                    width: 56
                                    height: 56
                                    radius: 10
                                    color: "#6D28D9"
                                    Image {
                                        anchors.centerIn: parent
                                        source: "assets/icon-window.svg"
                                        sourceSize.width: 28
                                        sourceSize.height: 28
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 4
                                    Text {
                                        text: qsTr("Pencere Yakala")
                                        color: "#F8FAFC"
                                        font.pixelSize: 16
                                        font.bold: true
                                    }
                                    Text {
                                        text: qsTr("Aktif pencereyi tek adımda çerçevesiyle yakalayın.")
                                        color: "#94A3B8"
                                        font.pixelSize: 12
                                        wrapMode: Text.WordWrap
                                        Layout.fillWidth: true
                                    }
                                    Rectangle {
                                        height: 22
                                        width: 100
                                        radius: 4
                                        color: "#334155"
                                        Text {
                                            anchors.centerIn: parent
                                            text: "Alt + PrtScr"
                                            color: "#E2E8F0"
                                            font.pixelSize: 11
                                        }
                                    }
                                }
                            }
                        }

                        // Card 4: Delayed Capture
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 140
                            color: delayMouse.containsMouse ? "#1E293B" : "#131D31"
                            border.color: delayMouse.containsMouse ? "#F59E0B" : "#334155"
                            border.width: 1.5
                            radius: 12

                            MouseArea {
                                id: delayMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    captureEngine.requestRegionCapture(5)
                                }
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 20
                                spacing: 16

                                Rectangle {
                                    width: 56
                                    height: 56
                                    radius: 10
                                    color: "#B45309"
                                    Image {
                                        anchors.centerIn: parent
                                        source: "assets/icon-timer.svg"
                                        sourceSize.width: 28
                                        sourceSize.height: 28
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 4
                                    Text {
                                        text: qsTr("5sn Gecikmeli Yakala")
                                        color: "#F8FAFC"
                                        font.pixelSize: 16
                                        font.bold: true
                                    }
                                    Text {
                                        text: qsTr("Menü ve açılır pencereleri hazırlamak için 5 saniye bekler.")
                                        color: "#94A3B8"
                                        font.pixelSize: 12
                                        wrapMode: Text.WordWrap
                                        Layout.fillWidth: true
                                    }
                                    Rectangle {
                                        height: 22
                                        width: 110
                                        radius: 4
                                        color: "#334155"
                                        Text {
                                            anchors.centerIn: parent
                                            text: "Ctrl + PrtScr"
                                            color: "#E2E8F0"
                                            font.pixelSize: 11
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // TAB 1: Library View
            LibraryView {
                id: libraryTab
            }

            // TAB 2: Settings View
            SettingsView {
                id: settingsTab
            }
        }
    }
}
