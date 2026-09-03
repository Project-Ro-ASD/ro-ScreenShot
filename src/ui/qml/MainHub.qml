import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Effects

ApplicationWindow {
    id: root
    visible: true
    width: 1080
    height: 720
    minimumWidth: 880
    minimumHeight: 600
    title: qsTr("Ro-ScreenShot Hub")

    property int currentTab: 0 // 0: Quick Capture, 1: Library, 2: Settings

    Shortcut {
        sequence: "Shift+Print"
        context: Qt.ApplicationShortcut
        onActivated: captureEngine.requestRegionCapture(0)
    }
    Shortcut {
        sequence: "Print"
        context: Qt.ApplicationShortcut
        onActivated: captureEngine.requestFullscreenCapture(0)
    }
    Shortcut {
        sequence: "Alt+Print"
        context: Qt.ApplicationShortcut
        onActivated: captureEngine.requestWindowCapture(0)
    }
    Shortcut {
        sequence: "Ctrl+Print"
        context: Qt.ApplicationShortcut
        onActivated: captureEngine.requestRegionCapture(5)
    }
    Shortcut {
        sequence: "Ctrl+Shift+Print"
        context: Qt.ApplicationShortcut
        onActivated: captureEngine.requestRegionCaptureWithAction(0, "copy")
    }
    Shortcut {
        sequence: "Ctrl+Alt+Print"
        context: Qt.ApplicationShortcut
        onActivated: captureEngine.requestRegionCaptureWithAction(0, "save")
    }
    Shortcut {
        sequence: "Ctrl+Shift+R"
        context: Qt.ApplicationShortcut
        enabled: captureEngine.hasLastRegion
        onActivated: captureEngine.requestLastRegionCapture(0)
    }
    Shortcut {
        sequence: "Ctrl+Shift+L"
        context: Qt.ApplicationShortcut
        onActivated: root.currentTab = 1
    }
    Shortcut {
        sequence: "Ctrl+,"
        context: Qt.ApplicationShortcut
        onActivated: root.currentTab = 2
    }

    readonly property bool hasUiPreferences: UiPreferencesManager !== null
    readonly property string themeMode: hasUiPreferences ? UiPreferencesManager.themeMode : "light"
    readonly property bool darkMode: themeMode === "dark"
    readonly property var visibleThemeModes: hasUiPreferences ? UiPreferencesManager.availableThemeModes : []
    readonly property var uiColors: colors

    QtObject {
        id: colors
        // Clean high-contrast palettes for light and dark themes
        readonly property color window:      root.darkMode ? "#0B1120" : "#F4F6F8"
        readonly property color shell:       root.darkMode ? "#131D31" : "#EAEFF4"
        readonly property color shellAlt:    root.darkMode ? "#0F172A" : "#E2E8F0"
        readonly property color card:        root.darkMode ? "#131D31" : "#FFFFFF"
        readonly property color cardStrong:  root.darkMode ? "#1E293B" : "#F1F5F9"
        readonly property color border:      root.darkMode ? "#334155" : "#CBD5E1"
        readonly property color text:        root.darkMode ? "#F8FAFC" : "#0F172A"
        readonly property color textMuted:   root.darkMode ? "#CBD5E1" : "#475569"
        readonly property color textSoft:    root.darkMode ? "#94A3B8" : "#64748B"
        readonly property color placeholder: root.darkMode ? "#64748B" : "#94A3B8"
        readonly property color muted:       root.darkMode ? "#475569" : "#8A94A6"
        readonly property color accentA:     root.darkMode ? "#2563EB" : "#2563EB"
        readonly property color accentB:     root.darkMode ? "#3B82F6" : "#3B82F6"
        readonly property color accentC:     root.darkMode ? "#1D4ED8" : "#1D4ED8"
        readonly property color selected:    root.darkMode ? "#1E3A8A" : "#DBEAFE"
        readonly property color success:     root.darkMode ? "#10B981" : "#059669"
        readonly property color warning:     root.darkMode ? "#F59E0B" : "#D97706"
        readonly property color danger:      root.darkMode ? "#EF4444" : "#DC2626"
    }

    color: colors.window
    Material.theme: darkMode ? Material.Dark : Material.Light
    Material.accent: colors.accentA
    Material.primary: colors.accentB
    Material.background: colors.window
    Material.foreground: colors.text

    property string captureStatusMessage: ""
    property bool captureStatusIsError: false

    function showCaptureStatus(message, isError) {
        captureStatusMessage = message
        captureStatusIsError = isError
        captureStatusPopup.open()
        captureStatusTimer.restart()
    }

    Connections {
        target: captureEngine
        function onCaptureSuccess(filePath, fileName, savedToDisk, copiedToClipboard) {
            var details = []
            if (savedToDisk)
                details.push(qsTr("Kaydedildi"))
            if (copiedToClipboard)
                details.push(qsTr("Panoya kopyalandı"))
            root.showCaptureStatus(details.length > 0 ? details.join(" • ") : qsTr("Ekran görüntüsü hazır"), false)
        }
        function onCaptureError(message) {
            root.showCaptureStatus(message, true)
        }
        function onColorCopied(hexColor) {
            root.showCaptureStatus(qsTr("Renk panoya kopyalandı: %1").arg(hexColor), false)
        }
    }

    Timer {
        id: captureStatusTimer
        interval: 5000
        repeat: false
        onTriggered: captureStatusPopup.close()
    }

    Popup {
        id: captureStatusPopup
        parent: Overlay.overlay
        x: parent.width - width - 24
        y: parent.height - height - 24
        width: Math.min(420, parent.width - 48)
        height: statusContent.implicitHeight + 28
        modal: false
        focus: false
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        background: Rectangle {
            radius: 10
            color: root.captureStatusIsError ? "#3B121C" : "#10372C"
            border.color: root.captureStatusIsError ? colors.danger : colors.success
            border.width: 1
        }
        contentItem: RowLayout {
            id: statusContent
            spacing: 10
            Text {
                text: root.captureStatusIsError ? "!" : "✓"
                color: root.captureStatusIsError ? "#FCA5A5" : "#6EE7B7"
                font.pixelSize: 18
                font.bold: true
            }
            Text {
                Layout.fillWidth: true
                text: root.captureStatusMessage
                color: "#F8FAFC"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
            }
            Button {
                flat: true
                Accessible.name: qsTr("Bildirimi kapat")
                onClicked: captureStatusPopup.close()
                contentItem: Text {
                    text: "×"
                    color: "#CBD5E1"
                    font.pixelSize: 18
                }
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // ─── Left Sidebar Navigation (no brand header, icon+label only) ─────────
        Rectangle {
            Layout.preferredWidth: 230
            Layout.fillHeight: true
            color: colors.shell
            border.color: colors.border
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
                        color: root.currentTab === 0 ? colors.accentA : (btn1Mouse.containsMouse ? colors.cardStrong : "transparent")

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
                                source: "qrc:/qt/qml/ro_screenshot/assets/icon-capture.svg"
                                sourceSize.width: 20
                                sourceSize.height: 20
                                Layout.preferredWidth: 20
                                Layout.preferredHeight: 20
                            }

                            Text {
                                text: qsTr("Hızlı Çekim")
                                color: root.currentTab === 0 ? "#FFFFFF" : colors.textSoft
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
                        color: root.currentTab === 1 ? colors.accentA : (btn2Mouse.containsMouse ? colors.cardStrong : "transparent")

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
                                source: "qrc:/qt/qml/ro_screenshot/assets/icon-gallery.svg"
                                sourceSize.width: 20
                                sourceSize.height: 20
                                Layout.preferredWidth: 20
                                Layout.preferredHeight: 20
                            }

                            Text {
                                text: qsTr("Galeri")
                                color: root.currentTab === 1 ? "#FFFFFF" : colors.textSoft
                                font.pixelSize: 13
                                font.bold: root.currentTab === 1
                                Layout.fillWidth: true
                            }

                            Rectangle {
                                height: 20
                                width: countBadgeText.implicitWidth + 10
                                radius: 10
                                color: root.currentTab === 1 ? colors.accentC : colors.cardStrong
                                Text {
                                    id: countBadgeText
                                    anchors.centerIn: parent
                                    text: libraryManager.count
                                    color: root.currentTab === 1 ? "#FFFFFF" : colors.textSoft
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
                        color: root.currentTab === 2 ? colors.accentA : (btn3Mouse.containsMouse ? colors.cardStrong : "transparent")

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
                                source: "qrc:/qt/qml/ro_screenshot/assets/icon-settings.svg"
                                sourceSize.width: 20
                                sourceSize.height: 20
                                Layout.preferredWidth: 20
                                Layout.preferredHeight: 20
                            }

                            Text {
                                text: qsTr("Ayarlar")
                                color: root.currentTab === 2 ? "#FFFFFF" : colors.textSoft
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
                        color: colors.border
                    }

                    // Language Row: native label toggle buttons (ro-Control style)
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 6

                        Text {
                            text: qsTr("Dil")
                            color: colors.textSoft
                            font.pixelSize: 11
                            font.bold: true
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 6

                            Repeater {
                                model: LanguageManager.availableLanguages
                                delegate: Button {
                                    required property var modelData
                                    Layout.fillWidth: true
                                    implicitHeight: 32
                                    text: modelData.nativeLabel
                                    checkable: true
                                    checked: LanguageManager.currentLanguage === modelData.code
                                    onClicked: LanguageManager.setCurrentLanguage(modelData.code)
                                    contentItem: Text {
                                        text: parent.text
                                        color: parent.checked ? "#FFFFFF" : colors.textMuted
                                        font.pixelSize: 11
                                        font.bold: parent.checked
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    background: Rectangle {
                                        radius: 6
                                        color: parent.checked ? colors.accentA : (parent.hovered ? colors.cardStrong : colors.shell)
                                        border.color: parent.checked ? colors.accentA : colors.border
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
                        color: colors.shellAlt
                        border.color: colors.border

                        RowLayout {
                            anchors.centerIn: parent
                            spacing: 6

                            Image {
                                source: "qrc:/qt/qml/ro_screenshot/assets/icon-folder.svg"
                                sourceSize.width: 14
                                sourceSize.height: 14
                                Layout.preferredWidth: 14
                                Layout.preferredHeight: 14
                            }

                            Text {
                                text: libraryManager.totalStorageSize
                                color: colors.textSoft
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
                            color: colors.text
                            font.pixelSize: 24
                            font.bold: true
                            Layout.alignment: Qt.AlignHCenter
                        }
                        Text {
                            text: qsTr("Bir çekim modu seçin veya sistem kısayollarını kullanın.")
                            color: colors.textSoft
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
                            color: regionMouse.containsMouse ? colors.cardStrong : colors.card
                            border.color: regionMouse.containsMouse ? colors.accentB : colors.border
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
                                    color: colors.accentC
                                    Image {
                                        anchors.centerIn: parent
                                        source: "qrc:/qt/qml/ro_screenshot/assets/icon-capture.svg"
                                        width: 28
                                        height: 28
                                        sourceSize.width: 28
                                        sourceSize.height: 28
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 4
                                    Text {
                                        text: qsTr("Bölge Seçimi (Sniper)")
                                        color: colors.text
                                        font.pixelSize: 16
                                        font.bold: true
                                    }
                                    Text {
                                        text: qsTr("Dondurulmuş ekranda serbest dikdörtgen alanı seçin ve kırpın.")
                                        color: colors.textSoft
                                        font.pixelSize: 12
                                        wrapMode: Text.WordWrap
                                        Layout.fillWidth: true
                                    }
                                    Rectangle {
                                        height: 22
                                        width: 120
                                        radius: 4
                                        color: colors.border
                                        Text {
                                            anchors.centerIn: parent
                                            text: "Shift + PrtScr"
                                            color: colors.textMuted
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
                            color: fullMouse.containsMouse ? colors.cardStrong : colors.card
                            border.color: fullMouse.containsMouse ? colors.success : colors.border
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
                                        source: "qrc:/qt/qml/ro_screenshot/assets/icon-fullscreen.svg"
                                        width: 28
                                        height: 28
                                        sourceSize.width: 28
                                        sourceSize.height: 28
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 4
                                    Text {
                                        text: qsTr("Tam Ekran Yakala")
                                        color: colors.text
                                        font.pixelSize: 16
                                        font.bold: true
                                    }
                                    Text {
                                        text: qsTr("Tüm monitörlerin görüntüsünü anında yakalayın ve kaydedin.")
                                        color: colors.textSoft
                                        font.pixelSize: 12
                                        wrapMode: Text.WordWrap
                                        Layout.fillWidth: true
                                    }
                                    Rectangle {
                                        height: 22
                                        width: 80
                                        radius: 4
                                        color: colors.border
                                        Text {
                                            anchors.centerIn: parent
                                            text: "PrtScr"
                                            color: colors.textMuted
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
                            color: winMouse.containsMouse ? colors.cardStrong : colors.card
                            border.color: winMouse.containsMouse ? "#8B5CF6" : colors.border
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
                                        source: "qrc:/qt/qml/ro_screenshot/assets/icon-window.svg"
                                        width: 28
                                        height: 28
                                        sourceSize.width: 28
                                        sourceSize.height: 28
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 4
                                    Text {
                                        text: qsTr("Pencere Yakala")
                                        color: colors.text
                                        font.pixelSize: 16
                                        font.bold: true
                                    }
                                    Text {
                                        text: qsTr("Aktif pencereyi tek adımda çerçevesiyle yakalayın.")
                                        color: colors.textSoft
                                        font.pixelSize: 12
                                        wrapMode: Text.WordWrap
                                        Layout.fillWidth: true
                                    }
                                    Rectangle {
                                        height: 22
                                        width: 100
                                        radius: 4
                                        color: colors.border
                                        Text {
                                            anchors.centerIn: parent
                                            text: "Alt + PrtScr"
                                            color: colors.textMuted
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
                            color: delayMouse.containsMouse ? colors.cardStrong : colors.card
                            border.color: delayMouse.containsMouse ? colors.warning : colors.border
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
                                        source: "qrc:/qt/qml/ro_screenshot/assets/icon-timer.svg"
                                        width: 28
                                        height: 28
                                        sourceSize.width: 28
                                        sourceSize.height: 28
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 4
                                    Text {
                                        text: qsTr("5sn Gecikmeli Yakala")
                                        color: colors.text
                                        font.pixelSize: 16
                                        font.bold: true
                                    }
                                    Text {
                                        text: qsTr("Menü ve açılır pencereleri hazırlamak için 5 saniye bekler.")
                                        color: colors.textSoft
                                        font.pixelSize: 12
                                        wrapMode: Text.WordWrap
                                        Layout.fillWidth: true
                                    }
                                    Rectangle {
                                        height: 22
                                        width: 110
                                        radius: 4
                                        color: colors.border
                                        Text {
                                            anchors.centerIn: parent
                                            text: "Ctrl + PrtScr"
                                            color: colors.textMuted
                                            font.pixelSize: 11
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 58
                        radius: 10
                        color: repeatRegionMouse.containsMouse && captureEngine.hasLastRegion
                               ? colors.selected : colors.card
                        border.width: 1
                        border.color: repeatRegionMouse.containsMouse && captureEngine.hasLastRegion
                                      ? colors.accentA : colors.border
                        opacity: captureEngine.hasLastRegion ? 1.0 : 0.55

                        MouseArea {
                            id: repeatRegionMouse
                            anchors.fill: parent
                            enabled: captureEngine.hasLastRegion && !captureEngine.isCapturing
                            hoverEnabled: true
                            cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                            onClicked: captureEngine.requestLastRegionCapture(0)
                        }

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 16
                            anchors.rightMargin: 16
                            spacing: 12

                            Image {
                                source: "qrc:/qt/qml/ro_screenshot/assets/icon-refresh.svg"
                                sourceSize.width: 20
                                sourceSize.height: 20
                                Layout.preferredWidth: 20
                                Layout.preferredHeight: 20
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 1
                                Text {
                                    text: qsTr("Son bölgeyi tekrar yakala")
                                    color: colors.text
                                    font.pixelSize: 13
                                    font.bold: true
                                }
                                Text {
                                    text: captureEngine.hasLastRegion
                                          ? qsTr("Önceki seçim sınırlarını yeniden kullanır.")
                                          : qsTr("İlk bölge çekiminden sonra kullanılabilir.")
                                    color: colors.textSoft
                                    font.pixelSize: 11
                                }
                            }

                            Text {
                                text: qsTr("Son seçim")
                                color: colors.textMuted
                                font.pixelSize: 10
                            }
                        }
                    }
                }
            }

            // TAB 1: Library View
            LibraryView {
                id: libraryTab
                colors: root.uiColors
            }

            // TAB 2: Settings View
            SettingsView {
                id: settingsTab
                colors: root.uiColors
            }
        }
    }
}
