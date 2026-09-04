import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import "components"

ApplicationWindow {
    id: root
    visible: true
    width: 1040
    height: 700
    minimumWidth: 900
    minimumHeight: 600
    title: qsTr("ro-ScreenShot")

    property int currentTab: 0

    Shortcut { sequence: "Shift+Print"; context: Qt.ApplicationShortcut; onActivated: captureEngine.requestRegionCapture(0) }
    Shortcut { sequence: "Print"; context: Qt.ApplicationShortcut; onActivated: captureEngine.requestFullscreenCapture(0) }
    Shortcut { sequence: "Alt+Print"; context: Qt.ApplicationShortcut; onActivated: captureEngine.requestWindowCapture(0) }
    Shortcut { sequence: "Ctrl+Print"; context: Qt.ApplicationShortcut; onActivated: captureEngine.requestRegionCapture(5) }
    Shortcut { sequence: "Ctrl+Shift+Print"; context: Qt.ApplicationShortcut; onActivated: captureEngine.requestRegionCaptureWithAction(0, "copy") }
    Shortcut { sequence: "Ctrl+Alt+Print"; context: Qt.ApplicationShortcut; onActivated: captureEngine.requestRegionCaptureWithAction(0, "save") }
    Shortcut { sequence: "Ctrl+Shift+R"; context: Qt.ApplicationShortcut; enabled: captureEngine.hasLastRegion; onActivated: captureEngine.requestLastRegionCapture(0) }
    Shortcut { sequence: "Ctrl+Shift+L"; context: Qt.ApplicationShortcut; onActivated: root.currentTab = 1 }
    Shortcut { sequence: "Ctrl+,"; context: Qt.ApplicationShortcut; onActivated: root.currentTab = 2 }
    Shortcut { sequence: "Ctrl+B"; context: Qt.ApplicationShortcut; onActivated: if (root.hasUiPreferences) UiPreferencesManager.toggleSidebar() }

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
                LanguageManager.setCurrentLanguage(langs[(i + 1) % langs.length].code)
                return
            }
        }
        LanguageManager.setCurrentLanguage(langs[0].code)
    }

    QtObject {
        id: colors
        readonly property color window:      root.darkMode ? "#0B0F19" : "#F7F8FA"
        readonly property color sidebar:     root.darkMode ? "#0F1524" : "#FFFFFF"
        readonly property color card:        root.darkMode ? "#151C2E" : "#FFFFFF"
        readonly property color cardHover:   root.darkMode ? "#1B2440" : "#F2F4F7"
        readonly property color accentSoft:  root.darkMode ? "#1E2A4A" : "#EFF4FF"
        readonly property color accentMuted: root.darkMode ? "#3B5BD6" : "#BFDBFE"
        readonly property color codeBg:      root.darkMode ? "#0B0F19" : "#F2F4F7"
        readonly property color shell:       root.darkMode ? "#0F1524" : "#FFFFFF"
        readonly property color shellAlt:    root.darkMode ? "#0B0F19" : "#F2F4F7"
        readonly property color cardStrong:  root.darkMode ? "#1B2440" : "#F2F4F7"
        readonly property color border:      root.darkMode ? "#232D45" : "#E6E8EC"
        readonly property color text:        root.darkMode ? "#F3F4F6" : "#111827"
        readonly property color textMuted:   root.darkMode ? "#CBD5E1" : "#4B5563"
        readonly property color textSoft:    root.darkMode ? "#94A3B8" : "#6B7280"
        readonly property color placeholder: root.darkMode ? "#64748B" : "#9CA3AF"
        readonly property color muted:       root.darkMode ? "#475569" : "#9CA3AF"
        readonly property color accent:      root.darkMode ? "#3B82F6" : "#2563EB"
        readonly property color accentA:     root.darkMode ? "#3B82F6" : "#2563EB"
        readonly property color accentB:     root.darkMode ? "#3B82F6" : "#2563EB"
        readonly property color accentC:     root.darkMode ? "#1D4ED8" : "#1D4ED8"
        readonly property color accentHover: root.darkMode ? "#60A5FA" : "#1D4ED8"
        readonly property color selected:    root.darkMode ? "#1E2A4A" : "#EFF4FF"
        readonly property color success:     root.darkMode ? "#10B981" : "#059669"
        readonly property color warning:     root.darkMode ? "#F59E0B" : "#D97706"
        readonly property color danger:      root.darkMode ? "#F87171" : "#DC2626"
    }

    color: colors.window
    Material.theme: darkMode ? Material.Dark : Material.Light
    Material.accent: colors.accent
    Material.primary: colors.accent
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
            if (savedToDisk) details.push(qsTr("Saved"))
            if (copiedToClipboard) details.push(qsTr("Copied to clipboard"))
            root.showCaptureStatus(details.length > 0 ? details.join("  ·  ") : qsTr("Screenshot ready"), false)
        }
        function onCaptureError(message) { root.showCaptureStatus(message, true) }
        function onColorCopied(hexColor) { root.showCaptureStatus(qsTr("Color copied: %1").arg(hexColor), false) }
    }

    Timer { id: captureStatusTimer; interval: 4000; repeat: false; onTriggered: captureStatusPopup.close() }

    Popup {
        id: captureStatusPopup
        parent: Overlay.overlay
        x: (parent.width - width) / 2
        y: parent.height - height - 28
        width: Math.min(440, parent.width - 48)
        height: statusContent.implicitHeight + 24
        modal: false
        focus: false
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        background: Rectangle {
            radius: 10
            color: root.darkMode ? "#151C2E" : "#111827"
            border.color: root.captureStatusIsError ? colors.danger : colors.success
            border.width: 1
        }
        contentItem: RowLayout {
            id: statusContent
            spacing: 10
            Rectangle {
                Layout.preferredWidth: 22; Layout.preferredHeight: 22; radius: 11
                color: root.captureStatusIsError ? colors.danger : colors.success
                Text {
                    anchors.centerIn: parent
                    text: root.captureStatusIsError ? "!" : "✓"
                    color: "#FFFFFF"; font.pixelSize: 12; font.bold: true
                }
            }
            Text {
                Layout.fillWidth: true
                text: root.captureStatusMessage
                color: "#F9FAFB"; font.pixelSize: 13
                wrapMode: Text.WordWrap; elide: Text.ElideRight
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // ── Sidebar ──
        Rectangle {
            Layout.preferredWidth: root.sidebarCollapsed ? 68 : 228
            Layout.fillHeight: true
            color: colors.sidebar
            border.color: colors.border
            border.width: 0
            clip: true
            Behavior on Layout.preferredWidth { NumberAnimation { duration: 160; easing.type: Easing.OutCubic } }

            Rectangle { anchors.right: parent.right; width: 1; height: parent.height; color: colors.border }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: root.sidebarCollapsed ? 10 : 14
                spacing: 6

                RowLayout {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
                    spacing: 8
                    Image {
                        source: "qrc:/qt/qml/ro_screenshot/assets/logo.svg"
                        sourceSize.width: 24; sourceSize.height: 24
                        Layout.preferredWidth: 24; Layout.preferredHeight: 24
                        Layout.alignment: root.sidebarCollapsed ? Qt.AlignHCenter : Qt.AlignVCenter
                    }
                    ColumnLayout {
                        visible: !root.sidebarCollapsed
                        Layout.fillWidth: true
                        spacing: 0
                        Text { text: "ro-ScreenShot"; color: colors.text; font.pixelSize: 13; font.bold: true; font.weight: Font.DemiBold }
                        Text { text: "v1.0 · Wayland"; color: colors.placeholder; font.pixelSize: 10 }
                    }
                    Rectangle {
                        visible: !root.sidebarCollapsed
                        Layout.preferredWidth: 28; Layout.preferredHeight: 28; radius: 7
                        color: collapseMouse.containsMouse ? colors.cardHover : "transparent"
                        Text { anchors.centerIn: parent; text: "‹"; color: colors.textSoft; font.pixelSize: 16; font.bold: true }
                        MouseArea { id: collapseMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: if (root.hasUiPreferences) UiPreferencesManager.toggleSidebar() }
                    }
                }

                Rectangle {
                    visible: root.sidebarCollapsed
                    Layout.preferredWidth: 32; Layout.preferredHeight: 28; Layout.alignment: Qt.AlignHCenter; radius: 7
                    color: expandMouse.containsMouse ? colors.cardHover : "transparent"
                    Text { anchors.centerIn: parent; text: "›"; color: colors.textSoft; font.pixelSize: 16; font.bold: true }
                    MouseArea { id: expandMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: if (root.hasUiPreferences) UiPreferencesManager.toggleSidebar() }
                }

                Item { Layout.preferredHeight: 4 }

                Text { visible: !root.sidebarCollapsed; text: qsTr("MENU"); color: colors.placeholder; font.pixelSize: 10; font.bold: true; font.letterSpacing: 1.2 }

                RoNavItem {
                    Layout.fillWidth: true
                    label: qsTr("Capture")
                    iconSource: "qrc:/qt/qml/ro_screenshot/assets/icon-capture.svg"
                    active: root.currentTab === 0
                    compact: root.sidebarCollapsed
                    colors: root.uiColors
                    onClicked: root.currentTab = 0
                }
                RoNavItem {
                    Layout.fillWidth: true
                    label: qsTr("Library")
                    iconSource: "qrc:/qt/qml/ro_screenshot/assets/icon-gallery.svg"
                    active: root.currentTab === 1
                    badge: libraryManager.count > 0 ? String(libraryManager.count) : ""
                    compact: root.sidebarCollapsed
                    colors: root.uiColors
                    onClicked: root.currentTab = 1
                }
                RoNavItem {
                    Layout.fillWidth: true
                    label: qsTr("Settings")
                    iconSource: "qrc:/qt/qml/ro_screenshot/assets/icon-settings.svg"
                    active: root.currentTab === 2
                    compact: root.sidebarCollapsed
                    colors: root.uiColors
                    onClicked: root.currentTab = 2
                }

                Item { Layout.fillHeight: true }

                // Language — compact 2x2 grid when expanded, single button when collapsed
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 6
                    visible: !root.sidebarCollapsed
                    Text { text: qsTr("LANGUAGE"); color: colors.placeholder; font.pixelSize: 10; font.bold: true; font.letterSpacing: 1.2 }
                    GridLayout {
                        Layout.fillWidth: true
                        columns: 2; rowSpacing: 6; columnSpacing: 6
                        Repeater {
                            model: LanguageManager.availableLanguages
                            delegate: Button {
                                required property var modelData
                                Layout.fillWidth: true
                                implicitHeight: 30
                                text: modelData.code.toUpperCase()
                                checkable: true
                                checked: LanguageManager.currentLanguage === modelData.code
                                onClicked: LanguageManager.setCurrentLanguage(modelData.code)
                                ToolTip.visible: hovered; ToolTip.delay: 400; ToolTip.text: modelData.nativeLabel
                                contentItem: Text { text: parent.text; color: parent.checked ? "#FFFFFF" : colors.textMuted; font.pixelSize: 11; font.bold: parent.checked; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                background: Rectangle { radius: 7; color: parent.checked ? colors.accent : (parent.hovered ? colors.cardHover : colors.codeBg); border.color: parent.checked ? "transparent" : colors.border; border.width: 1 }
                            }
                        }
                    }
                    // Theme segmented
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        Repeater {
                            model: UiPreferencesManager.availableThemeModes
                            delegate: Button {
                                required property var modelData
                                Layout.fillWidth: true
                                implicitHeight: 30
                                text: modelData.code === "light" ? qsTr("Light") : modelData.code === "dark" ? qsTr("Dark") : qsTr("Auto")
                                checkable: true
                                checked: UiPreferencesManager.themeMode === modelData.code
                                onClicked: UiPreferencesManager.setThemeMode(modelData.code)
                                contentItem: Text { text: parent.text; color: parent.checked ? "#FFFFFF" : colors.textMuted; font.pixelSize: 11; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                background: Rectangle { radius: 7; color: parent.checked ? colors.text : (parent.hovered ? colors.cardHover : colors.codeBg); border.color: parent.checked ? "transparent" : colors.border; border.width: 1 }
                            }
                        }
                    }
                }
                Rectangle {
                    visible: root.sidebarCollapsed
                    Layout.preferredWidth: 44; Layout.preferredHeight: 32; Layout.alignment: Qt.AlignHCenter; radius: 7
                    color: langMouse.containsMouse ? colors.cardHover : colors.codeBg
                    border.color: colors.border; border.width: 1
                    Text { anchors.centerIn: parent; text: LanguageManager.currentLanguage.toUpperCase(); color: colors.text; font.pixelSize: 11; font.bold: true }
                    MouseArea { id: langMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: root.cycleLanguage() }
                }
            }
        }

        // ── Content ──
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: root.currentTab

            ScrollView {
                clip: true
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                Flickable {
                    contentWidth: width
                    contentHeight: captureColumn.implicitHeight + 56
                    ColumnLayout {
                        id: captureColumn
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.top
                        anchors.topMargin: 28
                        spacing: 20
                        width: Math.min(parent.width - 64, 860)

                        RoPageHeader {
                            Layout.fillWidth: true
                            title: qsTr("Capture")
                            subtitle: qsTr("Choose a mode to start. Files save to your library automatically.")
                            statusText: qsTr("Portal ready")
                            colors: root.uiColors
                        }

                        GridLayout {
                            columns: 2; rowSpacing: 12; columnSpacing: 12
                            Layout.fillWidth: true
                            RoCaptureCard {
                                Layout.fillWidth: true
                                title: qsTr("Region")
                                description: qsTr("Drag to select any area with pixel loupe.")
                                shortcut: "Shift + PrtScr"
                                iconSource: "qrc:/qt/qml/ro_screenshot/assets/icon-crosshair.svg"
                                colors: root.uiColors
                                onClicked: captureEngine.requestRegionCapture(0)
                            }
                            RoCaptureCard {
                                Layout.fillWidth: true
                                title: qsTr("Fullscreen")
                                description: qsTr("Capture all connected monitors instantly.")
                                shortcut: "PrtScr"
                                iconSource: "qrc:/qt/qml/ro_screenshot/assets/icon-fullscreen.svg"
                                colors: root.uiColors
                                onClicked: captureEngine.requestFullscreenCapture(0)
                            }
                            RoCaptureCard {
                                Layout.fillWidth: true
                                title: qsTr("Window")
                                description: qsTr("Capture the focused window with frame.")
                                shortcut: "Alt + PrtScr"
                                iconSource: "qrc:/qt/qml/ro_screenshot/assets/icon-window.svg"
                                colors: root.uiColors
                                onClicked: captureEngine.requestWindowCapture(0)
                            }
                            RoCaptureCard {
                                Layout.fillWidth: true
                                title: qsTr("Delayed")
                                description: qsTr("5 second timer for menus and popovers.")
                                shortcut: "Ctrl + PrtScr"
                                iconSource: "qrc:/qt/qml/ro_screenshot/assets/icon-timer.svg"
                                colors: root.uiColors
                                onClicked: captureEngine.requestRegionCapture(5)
                            }
                        }

                        // Repeat last region — subtle single row
                        Rectangle {
                            Layout.fillWidth: true
                            implicitHeight: 46
                            radius: 10
                            color: repeatMouse.containsMouse && captureEngine.hasLastRegion ? colors.accentSoft : colors.card
                            border.color: repeatMouse.containsMouse && captureEngine.hasLastRegion ? colors.accent : colors.border
                            border.width: 1
                            opacity: captureEngine.hasLastRegion ? 1.0 : 0.5
                            MouseArea {
                                id: repeatMouse
                                anchors.fill: parent
                                enabled: captureEngine.hasLastRegion && !captureEngine.isCapturing
                                hoverEnabled: true
                                cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                                onClicked: captureEngine.requestLastRegionCapture(0)
                            }
                            RowLayout {
                                anchors.fill: parent; anchors.leftMargin: 14; anchors.rightMargin: 14; spacing: 10
                                Image { source: "qrc:/qt/qml/ro_screenshot/assets/icon-refresh.svg"; sourceSize.width: 15; sourceSize.height: 15; Layout.preferredWidth: 15; Layout.preferredHeight: 15; opacity: 0.7 }
                                Text { text: captureEngine.hasLastRegion ? qsTr("Repeat last region") : qsTr("Repeat last region — available after first capture"); color: colors.textMuted; font.pixelSize: 12; font.bold: captureEngine.hasLastRegion; Layout.fillWidth: true; elide: Text.ElideRight }
                                Text { visible: captureEngine.hasLastRegion; text: "Ctrl Shift R"; color: colors.placeholder; font.pixelSize: 10; font.family: "monospace" }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8
                            Layout.topMargin: 4
                            Text { text: qsTr("Studio"); color: colors.text; font.pixelSize: 14; font.bold: true; font.weight: Font.DemiBold }
                            Item { Layout.fillWidth: true }
                            Text { text: qsTr("%1 tools").arg(6); color: colors.placeholder; font.pixelSize: 11 }
                        }

                        GridLayout {
                            columns: 2; rowSpacing: 8; columnSpacing: 8
                            Layout.fillWidth: true
                            RoListRow { Layout.fillWidth: true; title: qsTr("Mockup Frame"); subtitle: qsTr("Device & gradient frames"); iconSource: "qrc:/qt/qml/ro_screenshot/assets/icon-window.svg"; colors: root.uiColors; onClicked: root.openMockupDialog(0) }
                            RoListRow { Layout.fillWidth: true; title: qsTr("Color & Table Kit"); subtitle: qsTr("Palette & markdown export"); iconSource: "qrc:/qt/qml/ro_screenshot/assets/icon-tags.svg"; colors: root.uiColors; onClicked: root.openDevKitDialog(0) }
                            RoListRow { Layout.fillWidth: true; title: qsTr("PDF Report"); subtitle: qsTr("Multi-shot binder"); iconSource: "qrc:/qt/qml/ro_screenshot/assets/icon-folder.svg"; colors: root.uiColors; onClicked: root.openPdfExportDialog() }
                            RoListRow { Layout.fillWidth: true; title: qsTr("Image Diff"); subtitle: qsTr("Side-by-side compare"); iconSource: "qrc:/qt/qml/ro_screenshot/assets/icon-search.svg"; colors: root.uiColors; onClicked: root.openImageDiffDialog() }
                            RoListRow { Layout.fillWidth: true; title: qsTr("Vault"); subtitle: qsTr("Encrypted private storage"); iconSource: "qrc:/qt/qml/ro_screenshot/assets/icon-settings.svg"; colors: root.uiColors; onClicked: root.openVaultDialog() }
                            RoListRow { Layout.fillWidth: true; title: qsTr("Present"); subtitle: qsTr("Laser pointer overlay"); iconSource: "qrc:/qt/qml/ro_screenshot/assets/icon-bolt.svg"; colors: root.uiColors; onClicked: { if (livePresentationOverlay) { livePresentationOverlay.togglePresentation(); root.showCaptureStatus(qsTr("Presentation mode on"), false) } } }
                        }
                        Item { height: 8 }
                    }
                }
            }

            LibraryView { id: libraryTab; colors: root.uiColors }
            SettingsView { id: settingsTab; colors: root.uiColors }
        }
    }

    DevKitDialog { id: globalDevKitDialog }
    MockupFrameDialog {
        id: globalMockupDialog
        onExportRequested: (row, preset, padding) => {
            var res = libraryManager.exportWithMockupFrame(row, preset, padding)
            if (res.length > 0) { root.showCaptureStatus(qsTr("Mockup saved"), false); libraryManager.refresh() }
        }
    }
    PdfExportDialog {
        id: globalPdfDialog
        selectedCount: libraryManager.selectedCount > 0 ? libraryManager.selectedCount : libraryManager.count
        onGenerateRequested: (title, notes, outPath) => {
            var target = outPath.length > 0 ? outPath : (settingsManager.saveDirectory + "/Report_" + Date.now() + ".pdf")
            if (libraryManager.generatePdfReportFromSelected(target, title, notes)) root.showCaptureStatus(qsTr("PDF created"), false)
        }
    }
    ImageDiffDialog { id: globalDiffDialog }
    VaultDialog {
        id: globalVaultDialog
        onUnlockRequested: (password) => {
            if (vaultManager && vaultManager.unlock(password)) { isUnlocked = true; statusText = qsTr("Vault unlocked."); root.showCaptureStatus(statusText, false) }
            else { statusText = qsTr("Wrong password."); root.showCaptureStatus(statusText, true) }
        }
        onLockRequested: { if (vaultManager) { vaultManager.lock(); isUnlocked = false; statusText = qsTr("Vault locked."); root.showCaptureStatus(statusText, false) } }
    }

    function openDevKitDialog(row) {
        if (libraryManager.count > 0) {
            var t = (row >= 0) ? row : 0
            globalDevKitDialog.paletteData = libraryManager.extractPaletteFromItem(t, 6)
            globalDevKitDialog.extractedTableText = libraryManager.extractTableFromItem(t, "markdown")
        }
        globalDevKitDialog.open()
    }
    function openMockupDialog(row) { globalMockupDialog.itemRow = (row >= 0 && row < libraryManager.count) ? row : 0; globalMockupDialog.open() }
    function openPdfExportDialog() { globalPdfDialog.open() }
    function openImageDiffDialog() {
        if (libraryManager.count >= 2) {
            var a = libraryManager.getItem(0), b = libraryManager.getItem(1)
            if (a && b) { globalDiffDialog.imagePathA = a.filePath; globalDiffDialog.imagePathB = b.filePath; globalDiffDialog.diffStats = libraryManager.compareSelectedImages() }
        }
        globalDiffDialog.open()
    }
    function openVaultDialog() { globalVaultDialog.open() }
}
