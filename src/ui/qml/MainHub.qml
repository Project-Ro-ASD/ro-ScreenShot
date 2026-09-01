import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

ApplicationWindow {
    id: root
    visible: true
    width: 1040
    height: 700
    minimumWidth: 860
    minimumHeight: 580
    title: qsTr("Ro-ScreenShot Hub")
    color: "#0F172A"

    property int currentTab: 0 // 0: Quick Capture, 1: Library, 2: Settings

    // Top Header Navigation Bar
    header: Rectangle {
        height: 64
        color: "#1E293B"
        border.color: "#334155"
        border.width: 1

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 20
            anchors.rightMargin: 20
            spacing: 16

            // Logo & Title
            RowLayout {
                spacing: 10
                Rectangle {
                    width: 36
                    height: 36
                    radius: 8
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#3B82F6" }
                        GradientStop { position: 1.0; color: "#1D4ED8" }
                    }
                    Text {
                        anchors.centerIn: parent
                        text: "📸"
                        font.pixelSize: 18
                    }
                }

                ColumnLayout {
                    spacing: 0
                    Text {
                        text: "Ro-ScreenShot"
                        color: "#F8FAFC"
                        font.pixelSize: 16
                        font.bold: true
                    }
                    Text {
                        text: qsTr("Project Ro-ASD • Fedora 44+")
                        color: "#94A3B8"
                        font.pixelSize: 11
                    }
                }
            }

            Item { Layout.fillWidth: true }

            // Navigation Tabs
            RowLayout {
                spacing: 6

                Button {
                    text: qsTr("🎯 Hızlı Çekim")
                    checkable: true
                    checked: root.currentTab === 0
                    onClicked: root.currentTab = 0
                    contentItem: Text {
                        text: parent.text
                        color: parent.checked ? "#FFFFFF" : "#94A3B8"
                        font.bold: parent.checked
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        implicitWidth: 120
                        implicitHeight: 38
                        color: parent.checked ? "#2563EB" : (parent.hovered ? "#334155" : "transparent")
                        radius: 6
                    }
                }

                Button {
                    text: qsTr("🖼️ Galeri (%1)").arg(libraryManager.count)
                    checkable: true
                    checked: root.currentTab === 1
                    onClicked: root.currentTab = 1
                    contentItem: Text {
                        text: parent.text
                        color: parent.checked ? "#FFFFFF" : "#94A3B8"
                        font.bold: parent.checked
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        implicitWidth: 130
                        implicitHeight: 38
                        color: parent.checked ? "#2563EB" : (parent.hovered ? "#334155" : "transparent")
                        radius: 6
                    }
                }

                Button {
                    text: qsTr("⚙️ Ayarlar")
                    checkable: true
                    checked: root.currentTab === 2
                    onClicked: root.currentTab = 2
                    contentItem: Text {
                        text: parent.text
                        color: parent.checked ? "#FFFFFF" : "#94A3B8"
                        font.bold: parent.checked
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        implicitWidth: 110
                        implicitHeight: 38
                        color: parent.checked ? "#2563EB" : (parent.hovered ? "#334155" : "transparent")
                        radius: 6
                    }
                }
            }
        }
    }

    // Main Content Area
    StackLayout {
        id: contentStack
        anchors.fill: parent
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

                // Grid of 4 Action Cards
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
                                Text {
                                    anchors.centerIn: parent
                                    text: "🎯"
                                    font.pixelSize: 28
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
                                Text {
                                    anchors.centerIn: parent
                                    text: "🖥️"
                                    font.pixelSize: 28
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
                                Text {
                                    anchors.centerIn: parent
                                    text: "🪟"
                                    font.pixelSize: 28
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
                                Text {
                                    anchors.centerIn: parent
                                    text: "⏱️"
                                    font.pixelSize: 28
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
