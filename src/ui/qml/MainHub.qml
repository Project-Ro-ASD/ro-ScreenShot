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
    title: qsTr("ro-ScreenShot Hub")

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
    Shortcut {
        sequence: "Ctrl+B"
        context: Qt.ApplicationShortcut
        onActivated: if (root.hasUiPreferences) UiPreferencesManager.toggleSidebar()
    }

    readonly property bool hasUiPreferences: UiPreferencesManager !== null
    readonly property bool sidebarCollapsed: hasUiPreferences ? UiPreferencesManager.sidebarCollapsed : false
    readonly property string themeMode: hasUiPreferences ? UiPreferencesManager.themeMode : "light"
    readonly property bool darkMode: themeMode === "dark"
    readonly property var visibleThemeModes: hasUiPreferences ? UiPreferencesManager.availableThemeModes : []
    readonly property var uiColors: colors

    function cycleLanguage() {
        var langs = LanguageManager.availableLanguages
        if (!langs || langs.length === 0) return
        for (var i = 0; i < langs.length; ++i) {
            if (langs[i].code === LanguageManager.currentLanguage) {
                var nextIndex = (i + 1) % langs.length
                LanguageManager.setCurrentLanguage(langs[nextIndex].code)
                return
            }
        }
        LanguageManager.setCurrentLanguage(langs[0].code)
    }

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

        // ─── Left Sidebar Navigation (Collapsible) ─────────
        Rectangle {
            id: sidebarContainer
            Layout.preferredWidth: root.sidebarCollapsed ? 64 : 230
            Layout.fillHeight: true
            color: colors.shell
            border.color: colors.border
            border.width: 1
            clip: true

            Behavior on Layout.preferredWidth {
                NumberAnimation {
                    duration: 180
                    easing.type: Easing.OutCubic
                }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: root.sidebarCollapsed ? 8 : 14
                spacing: 8

                // Header: App Logo & Collapse/Expand Toggle
                RowLayout {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
                    spacing: 8

                    // Brand Title (visible only when expanded)
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        visible: !root.sidebarCollapsed
                        opacity: root.sidebarCollapsed ? 0 : 1

                        Behavior on opacity {
                            NumberAnimation { duration: 120 }
                        }

                        Image {
                            source: "qrc:/qt/qml/ro_screenshot/assets/logo.svg"
                            sourceSize.width: 20
                            sourceSize.height: 20
                            Layout.preferredWidth: 20
                            Layout.preferredHeight: 20
                        }

                        Text {
                            text: "ro-ScreenShot"
                            color: colors.text
                            font.pixelSize: 13
                            font.bold: true
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }

                    // Toggle Button (Accessible in both expanded and collapsed modes)
                    Rectangle {
                        Layout.preferredWidth: 32
                        Layout.preferredHeight: 32
                        Layout.alignment: root.sidebarCollapsed ? Qt.AlignHCenter : Qt.AlignRight
                        radius: 6
                        color: toggleBtnMouse.containsMouse ? colors.cardStrong : "transparent"
                        border.color: toggleBtnMouse.containsMouse ? colors.border : "transparent"
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: root.sidebarCollapsed ? "☰" : "◀"
                            color: toggleBtnMouse.containsMouse ? colors.accentA : colors.textSoft
                            font.pixelSize: root.sidebarCollapsed ? 15 : 12
                            font.bold: true
                        }

                        MouseArea {
                            id: toggleBtnMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: if (root.hasUiPreferences) UiPreferencesManager.toggleSidebar()
                        }

                        ToolTip.visible: toggleBtnMouse.containsMouse
                        ToolTip.delay: 300
                        ToolTip.text: root.sidebarCollapsed ? qsTr("Kenar Çubuğunu Aç (Ctrl+B)") : qsTr("Kenar Çubuğunu Kapat (Ctrl+B)")
                    }
                }

                // Divider
                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: colors.border
                }

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

                        ToolTip.visible: root.sidebarCollapsed && btn1Mouse.containsMouse
                        ToolTip.delay: 300
                        ToolTip.text: qsTr("Hızlı Çekim")

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: root.sidebarCollapsed ? 0 : 12
                            anchors.rightMargin: root.sidebarCollapsed ? 0 : 12
                            spacing: root.sidebarCollapsed ? 0 : 12

                            Item {
                                Layout.preferredWidth: 20
                                Layout.preferredHeight: 20
                                Layout.alignment: root.sidebarCollapsed ? Qt.AlignHCenter : Qt.AlignVCenter

                                Image {
                                    anchors.centerIn: parent
                                    source: "qrc:/qt/qml/ro_screenshot/assets/icon-capture.svg"
                                    sourceSize.width: 20
                                    sourceSize.height: 20
                                }
                            }

                            Text {
                                text: qsTr("Hızlı Çekim")
                                color: root.currentTab === 0 ? "#FFFFFF" : colors.textSoft
                                font.pixelSize: 13
                                font.bold: root.currentTab === 0
                                Layout.fillWidth: true
                                visible: !root.sidebarCollapsed
                                opacity: root.sidebarCollapsed ? 0 : 1
                                elide: Text.ElideRight
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

                        ToolTip.visible: root.sidebarCollapsed && btn2Mouse.containsMouse
                        ToolTip.delay: 300
                        ToolTip.text: qsTr("Galeri (%1)").arg(libraryManager.count)

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: root.sidebarCollapsed ? 0 : 12
                            anchors.rightMargin: root.sidebarCollapsed ? 0 : 12
                            spacing: root.sidebarCollapsed ? 0 : 12

                            Item {
                                Layout.preferredWidth: 20
                                Layout.preferredHeight: 20
                                Layout.alignment: root.sidebarCollapsed ? Qt.AlignHCenter : Qt.AlignVCenter

                                Image {
                                    anchors.centerIn: parent
                                    source: "qrc:/qt/qml/ro_screenshot/assets/icon-gallery.svg"
                                    sourceSize.width: 20
                                    sourceSize.height: 20
                                }
                            }

                            Text {
                                text: qsTr("Galeri")
                                color: root.currentTab === 1 ? "#FFFFFF" : colors.textSoft
                                font.pixelSize: 13
                                font.bold: root.currentTab === 1
                                Layout.fillWidth: true
                                visible: !root.sidebarCollapsed
                                opacity: root.sidebarCollapsed ? 0 : 1
                                elide: Text.ElideRight
                            }

                            Rectangle {
                                visible: !root.sidebarCollapsed
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

                        ToolTip.visible: root.sidebarCollapsed && btn3Mouse.containsMouse
                        ToolTip.delay: 300
                        ToolTip.text: qsTr("Ayarlar")

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: root.sidebarCollapsed ? 0 : 12
                            anchors.rightMargin: root.sidebarCollapsed ? 0 : 12
                            spacing: root.sidebarCollapsed ? 0 : 12

                            Item {
                                Layout.preferredWidth: 20
                                Layout.preferredHeight: 20
                                Layout.alignment: root.sidebarCollapsed ? Qt.AlignHCenter : Qt.AlignVCenter

                                Image {
                                    anchors.centerIn: parent
                                    source: "qrc:/qt/qml/ro_screenshot/assets/icon-settings.svg"
                                    sourceSize.width: 20
                                    sourceSize.height: 20
                                }
                            }

                            Text {
                                text: qsTr("Ayarlar")
                                color: root.currentTab === 2 ? "#FFFFFF" : colors.textSoft
                                font.pixelSize: 13
                                font.bold: root.currentTab === 2
                                Layout.fillWidth: true
                                visible: !root.sidebarCollapsed
                                opacity: root.sidebarCollapsed ? 0 : 1
                                elide: Text.ElideRight
                            }
                        }
                    }
                }

                Item { Layout.fillHeight: true }

                // Bottom Sidebar: Language & Storage
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: colors.border
                    }

                    // Language Section (Expanded Mode)
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        visible: !root.sidebarCollapsed

                        Text {
                            text: qsTr("Dil")
                            color: colors.textSoft
                            font.pixelSize: 11
                            font.bold: true
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            Repeater {
                                model: LanguageManager.availableLanguages
                                delegate: Button {
                                    required property var modelData
                                    Layout.fillWidth: true
                                    implicitHeight: 30
                                    text: modelData.nativeLabel
                                    checkable: true
                                    checked: LanguageManager.currentLanguage === modelData.code
                                    onClicked: LanguageManager.setCurrentLanguage(modelData.code)
                                    contentItem: Text {
                                        text: parent.text
                                        color: parent.checked ? "#FFFFFF" : colors.textMuted
                                        font.pixelSize: 10
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

                    // Compact Language Button (Collapsed Mode)
                    Rectangle {
                        Layout.preferredWidth: 42
                        Layout.preferredHeight: 34
                        Layout.alignment: Qt.AlignHCenter
                        radius: 6
                        visible: root.sidebarCollapsed
                        color: langCollapsedMouse.containsMouse ? colors.accentA : colors.cardStrong
                        border.color: colors.border
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: LanguageManager.currentLanguage.toUpperCase()
                            color: langCollapsedMouse.containsMouse ? "#FFFFFF" : colors.text
                            font.pixelSize: 11
                            font.bold: true
                        }

                        MouseArea {
                            id: langCollapsedMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.cycleLanguage()
                        }

                        ToolTip.visible: langCollapsedMouse.containsMouse
                        ToolTip.delay: 300
                        ToolTip.text: qsTr("Dil: %1 (Değiştirmek için tıkla)").arg(LanguageManager.currentLanguageLabel)
                    }

                    // Storage Badge (Expanded Mode)
                    Rectangle {
                        Layout.fillWidth: true
                        height: 28
                        radius: 6
                        visible: !root.sidebarCollapsed
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

                    // Compact Storage Icon (Collapsed Mode)
                    Rectangle {
                        Layout.preferredWidth: 42
                        Layout.preferredHeight: 28
                        Layout.alignment: Qt.AlignHCenter
                        radius: 6
                        visible: root.sidebarCollapsed
                        color: storageCollapsedMouse.containsMouse ? colors.cardStrong : colors.shellAlt
                        border.color: colors.border

                        Image {
                            anchors.centerIn: parent
                            source: "qrc:/qt/qml/ro_screenshot/assets/icon-folder.svg"
                            sourceSize.width: 14
                            sourceSize.height: 14
                        }

                        MouseArea {
                            id: storageCollapsedMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: settingsManager.openSaveDirectory()
                        }

                        ToolTip.visible: storageCollapsedMouse.containsMouse
                        ToolTip.delay: 300
                        ToolTip.text: qsTr("Kayıt Alanı: %1").arg(libraryManager.totalStorageSize)
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
            ScrollView {
                id: quickCaptureScroll
                clip: true
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                ScrollBar.vertical.policy: ScrollBar.AsNeeded

                Flickable {
                    contentWidth: width
                    contentHeight: captureColumn.implicitHeight + 48

                    ColumnLayout {
                        id: captureColumn
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.top
                        anchors.topMargin: 24
                        spacing: 28
                        width: Math.min(parent.width - 48, 880)

                        // Hero Banner with subtle glowing gradient
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 92
                            radius: 14
                            color: root.darkMode ? "#131D31" : "#FFFFFF"
                            border.color: root.darkMode ? "#1E293B" : "#E2E8F0"
                            border.width: 1

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 18
                                spacing: 16

                                Rectangle {
                                    width: 48
                                    height: 48
                                    radius: 12
                                    color: colors.accentA

                                    Text {
                                        anchors.centerIn: parent
                                        text: "📸"
                                        font.pixelSize: 22
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 3
                                    Text {
                                        text: qsTr("Ekran Görüntüsü & Prodüksiyon Merkezi")
                                        color: colors.text
                                        font.pixelSize: 18
                                        font.bold: true
                                    }
                                    Text {
                                        text: qsTr("Hızlı çekim modlarını başlatın veya tasarım & geliştirici araçlarını kullanın.")
                                        color: colors.textSoft
                                        font.pixelSize: 12
                                    }
                                }

                                Rectangle {
                                    height: 28
                                    radius: 14
                                    color: root.darkMode ? "#1E293B" : "#F1F5F9"
                                    border.color: colors.border
                                    border.width: 1
                                    implicitWidth: statusPillRow.implicitWidth + 18

                                    RowLayout {
                                        id: statusPillRow
                                        anchors.centerIn: parent
                                        spacing: 6
                                        Rectangle {
                                            width: 8; height: 8; radius: 4
                                            color: "#10B981"
                                        }
                                        Text {
                                            text: qsTr("Wayland / Portal Aktif")
                                            color: colors.textMuted
                                            font.pixelSize: 11
                                            font.bold: true
                                        }
                                    }
                                }
                            }
                        }

                        // Grid of 4 Primary Capture Cards
                        GridLayout {
                            columns: 2
                            rowSpacing: 16
                            columnSpacing: 16
                            Layout.fillWidth: true

                            // Card 1: Region (Sniper)
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 128
                                color: regionMouse.containsMouse ? colors.cardStrong : colors.card
                                border.color: regionMouse.containsMouse ? colors.accentB : colors.border
                                border.width: regionMouse.containsMouse ? 2 : 1
                                radius: 12

                                MouseArea {
                                    id: regionMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: captureEngine.requestRegionCapture(0)
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 18
                                    spacing: 16

                                    Rectangle {
                                        width: 52
                                        height: 52
                                        radius: 12
                                        color: colors.accentC
                                        Image {
                                            anchors.centerIn: parent
                                            source: "qrc:/qt/qml/ro_screenshot/assets/icon-capture.svg"
                                            sourceSize.width: 26
                                            sourceSize.height: 26
                                        }
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 4
                                        Text {
                                            text: qsTr("Bölge Seçimi (Sniper)")
                                            color: colors.text
                                            font.pixelSize: 15
                                            font.bold: true
                                        }
                                        Text {
                                            text: qsTr("Büyüteç loupe ile piksel hassasiyetinde serbest dikdörtgen kırpın.")
                                            color: colors.textSoft
                                            font.pixelSize: 11
                                            wrapMode: Text.WordWrap
                                            Layout.fillWidth: true
                                        }
                                        Rectangle {
                                            height: 20
                                            implicitWidth: rKeyText.implicitWidth + 14
                                            radius: 4
                                            color: colors.border
                                            Text {
                                                id: rKeyText
                                                anchors.centerIn: parent
                                                text: "Shift + PrtScr"
                                                color: colors.textMuted
                                                font.pixelSize: 10
                                                font.bold: true
                                            }
                                        }
                                    }
                                }
                            }

                            // Card 2: Fullscreen
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 128
                                color: fullMouse.containsMouse ? colors.cardStrong : colors.card
                                border.color: fullMouse.containsMouse ? colors.success : colors.border
                                border.width: fullMouse.containsMouse ? 2 : 1
                                radius: 12

                                MouseArea {
                                    id: fullMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: captureEngine.requestFullscreenCapture(0)
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 18
                                    spacing: 16

                                    Rectangle {
                                        width: 52
                                        height: 52
                                        radius: 12
                                        color: "#047857"
                                        Image {
                                            anchors.centerIn: parent
                                            source: "qrc:/qt/qml/ro_screenshot/assets/icon-fullscreen.svg"
                                            sourceSize.width: 26
                                            sourceSize.height: 26
                                        }
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 4
                                        Text {
                                            text: qsTr("Tam Ekran Yakala")
                                            color: colors.text
                                            font.pixelSize: 15
                                            font.bold: true
                                        }
                                        Text {
                                            text: qsTr("Bağlı tüm monitörlerin görüntüsünü anında dondurup kaydedin.")
                                            color: colors.textSoft
                                            font.pixelSize: 11
                                            wrapMode: Text.WordWrap
                                            Layout.fillWidth: true
                                        }
                                        Rectangle {
                                            height: 20
                                            implicitWidth: fKeyText.implicitWidth + 14
                                            radius: 4
                                            color: colors.border
                                            Text {
                                                id: fKeyText
                                                anchors.centerIn: parent
                                                text: "PrtScr"
                                                color: colors.textMuted
                                                font.pixelSize: 10
                                                font.bold: true
                                            }
                                        }
                                    }
                                }
                            }

                            // Card 3: Window
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 128
                                color: winMouse.containsMouse ? colors.cardStrong : colors.card
                                border.color: winMouse.containsMouse ? "#8B5CF6" : colors.border
                                border.width: winMouse.containsMouse ? 2 : 1
                                radius: 12

                                MouseArea {
                                    id: winMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: captureEngine.requestWindowCapture(0)
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 18
                                    spacing: 16

                                    Rectangle {
                                        width: 52
                                        height: 52
                                        radius: 12
                                        color: "#6D28D9"
                                        Image {
                                            anchors.centerIn: parent
                                            source: "qrc:/qt/qml/ro_screenshot/assets/icon-window.svg"
                                            sourceSize.width: 26
                                            sourceSize.height: 26
                                        }
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 4
                                        Text {
                                            text: qsTr("Pencere Yakala")
                                            color: colors.text
                                            font.pixelSize: 15
                                            font.bold: true
                                        }
                                        Text {
                                            text: qsTr("Aktif pencereyi tek adımda kenarlık ve gölgesiyle yakalayın.")
                                            color: colors.textSoft
                                            font.pixelSize: 11
                                            wrapMode: Text.WordWrap
                                            Layout.fillWidth: true
                                        }
                                        Rectangle {
                                            height: 20
                                            implicitWidth: wKeyText.implicitWidth + 14
                                            radius: 4
                                            color: colors.border
                                            Text {
                                                id: wKeyText
                                                anchors.centerIn: parent
                                                text: "Alt + PrtScr"
                                                color: colors.textMuted
                                                font.pixelSize: 10
                                                font.bold: true
                                            }
                                        }
                                    }
                                }
                            }

                            // Card 4: Delayed Capture
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 128
                                color: delayMouse.containsMouse ? colors.cardStrong : colors.card
                                border.color: delayMouse.containsMouse ? colors.warning : colors.border
                                border.width: delayMouse.containsMouse ? 2 : 1
                                radius: 12

                                MouseArea {
                                    id: delayMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: captureEngine.requestRegionCapture(5)
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 18
                                    spacing: 16

                                    Rectangle {
                                        width: 52
                                        height: 52
                                        radius: 12
                                        color: "#B45309"
                                        Image {
                                            anchors.centerIn: parent
                                            source: "qrc:/qt/qml/ro_screenshot/assets/icon-timer.svg"
                                            sourceSize.width: 26
                                            sourceSize.height: 26
                                        }
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 4
                                        Text {
                                            text: qsTr("5sn Gecikmeli Yakala")
                                            color: colors.text
                                            font.pixelSize: 15
                                            font.bold: true
                                        }
                                        Text {
                                            text: qsTr("Menü ve açılır pencereleri açmak için 5 saniye sayaç başlatır.")
                                            color: colors.textSoft
                                            font.pixelSize: 11
                                            wrapMode: Text.WordWrap
                                            Layout.fillWidth: true
                                        }
                                        Rectangle {
                                            height: 20
                                            implicitWidth: dKeyText.implicitWidth + 14
                                            radius: 4
                                            color: colors.border
                                            Text {
                                                id: dKeyText
                                                anchors.centerIn: parent
                                                text: "Ctrl + PrtScr"
                                                color: colors.textMuted
                                                font.pixelSize: 10
                                                font.bold: true
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        // Last Region Repeat Bar
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 52
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
                                    sourceSize.width: 18
                                    sourceSize.height: 18
                                    Layout.preferredWidth: 18
                                    Layout.preferredHeight: 18
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 1
                                    Text {
                                        text: qsTr("Son bölgeyi tekrar yakala")
                                        color: colors.text
                                        font.pixelSize: 12
                                        font.bold: true
                                    }
                                    Text {
                                        text: captureEngine.hasLastRegion
                                              ? qsTr("Önceki seçim koordinatlarını aynen dondurur.")
                                              : qsTr("İlk bölge çekiminden sonra aktifleşir.")
                                        color: colors.textSoft
                                        font.pixelSize: 10
                                    }
                                }

                                Text {
                                    text: "Ctrl + Shift + R"
                                    color: colors.textMuted
                                    font.pixelSize: 10
                                    font.bold: true
                                }
                            }
                        }

                        // Section Title: Designer & Developer Suite
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8
                            Text {
                                text: qsTr("🛠️ Mühendislik & Tasarım Araç Kiti")
                                color: colors.text
                                font.pixelSize: 16
                                font.bold: true
                            }
                            Item { Layout.fillWidth: true }
                            Text {
                                text: qsTr("8 Akıllı Araç")
                                color: colors.accentB
                                font.pixelSize: 12
                                font.bold: true
                            }
                        }

                        // Grid of 6 Innovative Tool Cards
                        GridLayout {
                            columns: 3
                            rowSpacing: 12
                            columnSpacing: 12
                            Layout.fillWidth: true

                            // Tool 1: Mockup Frame
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 74
                                radius: 10
                                color: tool1M.containsMouse ? colors.cardStrong : colors.card
                                border.color: tool1M.containsMouse ? colors.accentA : colors.border
                                border.width: 1

                                MouseArea {
                                    id: tool1M
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: root.openMockupDialog(0)
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 10

                                    Rectangle {
                                        width: 36; height: 36; radius: 8
                                        color: "#3B82F6"
                                        Text { anchors.centerIn: parent; text: "🖼️"; font.pixelSize: 16 }
                                    }
                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2
                                        Text { text: qsTr("Mockup Çerçevesi"); color: colors.text; font.pixelSize: 12; font.bold: true }
                                        Text { text: qsTr("Sosyal medya gradyanları"); color: colors.textSoft; font.pixelSize: 10 }
                                    }
                                }
                            }

                            // Tool 2: DevKit (Palette & Table)
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 74
                                radius: 10
                                color: tool2M.containsMouse ? colors.cardStrong : colors.card
                                border.color: tool2M.containsMouse ? colors.accentA : colors.border
                                border.width: 1

                                MouseArea {
                                    id: tool2M
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: root.openDevKitDialog(0)
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 10

                                    Rectangle {
                                        width: 36; height: 36; radius: 8
                                        color: "#10B981"
                                        Text { anchors.centerIn: parent; text: "🎨"; font.pixelSize: 16 }
                                    }
                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2
                                        Text { text: qsTr("Renk & Tablo Kiti"); color: colors.text; font.pixelSize: 12; font.bold: true }
                                        Text { text: qsTr("Tailwind CSS & Markdown"); color: colors.textSoft; font.pixelSize: 10 }
                                    }
                                }
                            }

                            // Tool 3: PDF Export
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 74
                                radius: 10
                                color: tool3M.containsMouse ? colors.cardStrong : colors.card
                                border.color: tool3M.containsMouse ? colors.accentA : colors.border
                                border.width: 1

                                MouseArea {
                                    id: tool3M
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: root.openPdfExportDialog()
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 10

                                    Rectangle {
                                        width: 36; height: 36; radius: 8
                                        color: "#EF4444"
                                        Text { anchors.centerIn: parent; text: "📑"; font.pixelSize: 16 }
                                    }
                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2
                                        Text { text: qsTr("PDF Raporlayıcı"); color: colors.text; font.pixelSize: 12; font.bold: true }
                                        Text { text: qsTr("Çoklu çekim kılavuz binder"); color: colors.textSoft; font.pixelSize: 10 }
                                    }
                                }
                            }

                            // Tool 4: Image Diff
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 74
                                radius: 10
                                color: tool4M.containsMouse ? colors.cardStrong : colors.card
                                border.color: tool4M.containsMouse ? colors.accentA : colors.border
                                border.width: 1

                                MouseArea {
                                    id: tool4M
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: root.openImageDiffDialog()
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 10

                                    Rectangle {
                                        width: 36; height: 36; radius: 8
                                        color: "#8B5CF6"
                                        Text { anchors.centerIn: parent; text: "🔍"; font.pixelSize: 16 }
                                    }
                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2
                                        Text { text: qsTr("Görsel Diff"); color: colors.text; font.pixelSize: 12; font.bold: true }
                                        Text { text: qsTr("Split slider karşılaştırma"); color: colors.textSoft; font.pixelSize: 10 }
                                    }
                                }
                            }

                            // Tool 5: Secure Vault
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 74
                                radius: 10
                                color: tool5M.containsMouse ? colors.cardStrong : colors.card
                                border.color: tool5M.containsMouse ? colors.accentA : colors.border
                                border.width: 1

                                MouseArea {
                                    id: tool5M
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: root.openVaultDialog()
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 10

                                    Rectangle {
                                        width: 36; height: 36; radius: 8
                                        color: "#F59E0B"
                                        Text { anchors.centerIn: parent; text: "🔒"; font.pixelSize: 16 }
                                    }
                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2
                                        Text { text: qsTr("Gizlilik Kasası"); color: colors.text; font.pixelSize: 12; font.bold: true }
                                        Text { text: qsTr("AES-256 şifreli saklama"); color: colors.textSoft; font.pixelSize: 10 }
                                    }
                                }
                            }

                            // Tool 6: Presentation Laser
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 74
                                radius: 10
                                color: tool6M.containsMouse ? colors.cardStrong : colors.card
                                border.color: tool6M.containsMouse ? colors.accentA : colors.border
                                border.width: 1

                                MouseArea {
                                    id: tool6M
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        if (livePresentationOverlay) {
                                            livePresentationOverlay.togglePresentation();
                                            root.showCaptureStatus(qsTr("Canlı Sunum & Lazer Modu Açıldı"), false);
                                        }
                                    }
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 10

                                    Rectangle {
                                        width: 36; height: 36; radius: 8
                                        color: "#EC4899"
                                        Text { anchors.centerIn: parent; text: "🪄"; font.pixelSize: 16 }
                                    }
                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2
                                        Text { text: qsTr("Canlı Lazer Sunum"); color: colors.text; font.pixelSize: 12; font.bold: true }
                                        Text { text: qsTr("İmleç izi & tuş gösterici"); color: colors.textSoft; font.pixelSize: 10 }
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
                colors: root.uiColors
            }

            // TAB 2: Settings View
            SettingsView {
                id: settingsTab
                colors: root.uiColors
            }
        }
    }

    // ─── Global Dialogs & Modals ──────────────────────────────────────────────
    DevKitDialog {
        id: globalDevKitDialog
    }

    MockupFrameDialog {
        id: globalMockupDialog
        onExportRequested: (row, preset, padding) => {
            var res = libraryManager.exportWithMockupFrame(row, preset, padding);
            if (res.length > 0) {
                root.showCaptureStatus(qsTr("Mockup kaydedildi: %1").arg(res), false);
                libraryManager.refresh();
            }
        }
    }

    PdfExportDialog {
        id: globalPdfDialog
        selectedCount: libraryManager.selectedCount > 0 ? libraryManager.selectedCount : libraryManager.count
        onGenerateRequested: (title, notes, outPath) => {
            var target = outPath.length > 0 ? outPath : (settingsManager.saveDirectory + "/Report_" + Date.now() + ".pdf");
            if (libraryManager.generatePdfReportFromSelected(target, title, notes)) {
                root.showCaptureStatus(qsTr("PDF başarıyla oluşturuldu: %1").arg(target), false);
            }
        }
    }

    ImageDiffDialog {
        id: globalDiffDialog
    }

    VaultDialog {
        id: globalVaultDialog
        onUnlockRequested: (password) => {
            if (vaultManager && vaultManager.unlock(password)) {
                isUnlocked = true;
                statusText = qsTr("Kasa başarıyla açıldı.");
                root.showCaptureStatus(statusText, false);
            } else {
                statusText = qsTr("Hatalı parola!");
                root.showCaptureStatus(statusText, true);
            }
        }
        onLockRequested: {
            if (vaultManager) {
                vaultManager.lock();
                isUnlocked = false;
                statusText = qsTr("Kasa kilitlendi.");
                root.showCaptureStatus(statusText, false);
            }
        }
    }

    function openDevKitDialog(row) {
        if (libraryManager.count > 0) {
            var targetRow = (row >= 0) ? row : 0;
            globalDevKitDialog.paletteData = libraryManager.extractPaletteFromItem(targetRow, 6);
            globalDevKitDialog.extractedTableText = libraryManager.extractTableFromItem(targetRow, "markdown");
        }
        globalDevKitDialog.open();
    }

    function openMockupDialog(row) {
        globalMockupDialog.itemRow = (row >= 0 && row < libraryManager.count) ? row : 0;
        globalMockupDialog.open();
    }

    function openPdfExportDialog() {
        globalPdfDialog.open();
    }

    function openImageDiffDialog() {
        if (libraryManager.count >= 2) {
            var item0 = libraryManager.getItem(0);
            var item1 = libraryManager.getItem(1);
            if (item0 && item1) {
                globalDiffDialog.imagePathA = item0.filePath;
                globalDiffDialog.imagePathB = item1.filePath;
                globalDiffDialog.diffStats = libraryManager.compareSelectedImages();
            }
        }
        globalDiffDialog.open();
    }

    function openVaultDialog() {
        globalVaultDialog.open();
    }
}
