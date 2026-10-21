import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import "components"

ApplicationWindow {
    id: root
    visible: true
    width: 1180
    height: 760
    minimumWidth: 820
    minimumHeight: 600
    title: qsTr("ro-ScreenShot")

    property int currentTab: 0
    readonly property bool hasUiPreferences: UiPreferencesManager !== null
    readonly property bool darkMode: hasUiPreferences && UiPreferencesManager.themeMode === "dark"
    readonly property var uiColors: RoPalette
    readonly property var paletteColors: RoPalette
    property string statusMessage: ""
    property bool statusIsError: false


    color: paletteColors.canvas
    font.pixelSize: 13
    Material.theme: darkMode ? Material.Dark : Material.Light
    Material.accent: paletteColors.accent
    Material.primary: paletteColors.accent

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

    function showStatus(message, isError) {
        statusMessage = message;
        statusIsError = isError;
        statusTimer.restart();
    }

    Connections {
        target: captureEngine
        function onCaptureSuccess(filePath, fileName, savedToDisk, copiedToClipboard) {
            var destinations = [];
            if (savedToDisk)
                destinations.push(qsTr("Saved to library"));
            if (copiedToClipboard)
                destinations.push(qsTr("Copied"));
            root.showStatus(destinations.length ? destinations.join(" · ") : qsTr("Capture ready"), false);
        }
        function onCaptureError(message) {
            root.showStatus(message, true);
        }
        function onColorCopied(hexColor) {
            root.showStatus(qsTr("Color %1 copied").arg(hexColor), false);
        }
    }
    Timer {
        id: statusTimer
        interval: 4200
        onTriggered: root.statusMessage = ""
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 10

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 54
            radius: 14
            color: paletteColors.shell
            border.color: paletteColors.border
            border.width: 1
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 12
                spacing: 12
                Image {
                    source: "qrc:/qt/qml/ro_screenshot/assets/logo.svg"
                    sourceSize.width: 24
                    sourceSize.height: 24
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 24
                }
                Text {
                    text: "ro-ScreenShot"
                    color: paletteColors.text
                    font.pixelSize: 15
                    font.weight: Font.DemiBold
                }
                Text {
                    text: qsTr("Capture workspace")
                    color: paletteColors.textSoft
                    font.pixelSize: 12
                    Layout.fillWidth: true
                }
                Rectangle {
                    Layout.preferredWidth: 8
                    Layout.preferredHeight: 8
                    radius: 4
                    color: paletteColors.success
                }
                Text {
                    text: qsTr("Ready")
                    color: paletteColors.textMuted
                    font.pixelSize: 12
                }
                AccessibleIconButton {
                    iconSource: "qrc:/qt/qml/ro_screenshot/assets/icon-theme.svg"
                    tooltipText: qsTr("Change theme")
                    colors: paletteColors
                    onClicked: {
                        var modes = UiPreferencesManager.availableThemeModes;
                        for (var i = 0; i < modes.length; ++i)
                            if (modes[i].code === UiPreferencesManager.themeMode) {
                                UiPreferencesManager.setThemeMode(modes[(i + 1) % modes.length].code);
                                return;
                            }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12
            Rectangle {
                Layout.preferredWidth: 176
                Layout.fillHeight: true
                radius: RoTokens.radiusLg
                color: paletteColors.sidebar
                border.color: paletteColors.border
                border.width: 1
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 6
                    Text {
                        text: qsTr("WORKSPACE")
                        color: paletteColors.textMuted
                        font.pixelSize: 10
                        font.weight: Font.DemiBold
                        font.letterSpacing: 1.1
                        Layout.leftMargin: 8
                        Layout.topMargin: 8
                    }
                    NavButton {
                        label: qsTr("Capture")
                        glyphSource: "qrc:/qt/qml/ro_screenshot/assets/icon-capture.svg"
                        active: root.currentTab === 0
                        colors: paletteColors
                        onClicked: root.currentTab = 0
                    }
                    NavButton {
                        label: qsTr("Library")
                        glyphSource: "qrc:/qt/qml/ro_screenshot/assets/icon-gallery.svg"
                        badge: libraryManager.count > 0 ? libraryManager.count : 0
                        active: root.currentTab === 1
                        colors: paletteColors
                        onClicked: root.currentTab = 1
                    }
                    NavButton {
                        label: qsTr("Settings")
                        glyphSource: "qrc:/qt/qml/ro_screenshot/assets/icon-settings.svg"
                        active: root.currentTab === 2
                        colors: paletteColors
                        onClicked: root.currentTab = 2
                    }
                    Item {
                        Layout.fillHeight: true
                    }
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: paletteColors.border
                    }
                    Text {
                        text: qsTr("QUICK KEYS")
                        color: paletteColors.textMuted
                        font.pixelSize: 10
                        font.weight: Font.DemiBold
                        font.letterSpacing: 1.1
                        Layout.leftMargin: 8
                        Layout.topMargin: 8
                    }
                    KeyHint {
                        title: qsTr("Region")
                        key: "⇧ PrtSc"
                        colors: paletteColors
                    }
                    KeyHint {
                        title: qsTr("Screen")
                        key: "PrtSc"
                        colors: paletteColors
                    }
                    KeyHint {
                        title: qsTr("Window")
                        key: "⌥ PrtSc"
                        colors: paletteColors
                    }
                    Item {
                        Layout.preferredHeight: 4
                    }
                }
            }

            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: root.currentTab
                Item {
                    ScrollView {
                        anchors.fill: parent
                        clip: true
                        contentWidth: availableWidth
                        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                        ColumnLayout {
                            width: Math.min(parent.width, 1000)
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: 14
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 252
                                radius: 20
                                color: paletteColors.card
                                border.color: paletteColors.border
                                border.width: 1
                                clip: true
                                Rectangle {
                                    width: parent.width * 0.46
                                    height: parent.height * 1.45
                                    x: parent.width * 0.65
                                    y: -parent.height * 0.2
                                    radius: width / 2
                                    rotation: -18
                                    color: paletteColors.accentSoft
                                    opacity: 0.9
                                }
                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 28
                                    spacing: 24
                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 8
                                        Text {
                                            text: qsTr("CAPTURE STUDIO")
                                            color: paletteColors.accent
                                            font.pixelSize: 11
                                            font.weight: Font.DemiBold
                                            font.letterSpacing: 1.5
                                        }
                                        Text {
                                            text: qsTr("Frame exactly what matters.")
                                            color: paletteColors.text
                                            font.pixelSize: 29
                                            font.weight: Font.DemiBold
                                            wrapMode: Text.WordWrap
                                            Layout.maximumWidth: 440
                                        }
                                        Text {
                                            text: qsTr("Select a region, grab a display, or save a focused window. Your workflow stays one keystroke away.")
                                            color: paletteColors.textMuted
                                            font.pixelSize: 13
                                            wrapMode: Text.WordWrap
                                            Layout.maximumWidth: 470
                                        }
                                        Item {
                                            Layout.preferredHeight: 4
                                        }
                                        RoButton {
                                            text: qsTr("New region capture")
                                            variant: "primary"
                                            colors: paletteColors
                                            implicitWidth: 188
                                            implicitHeight: 40
                                            onClicked: captureEngine.requestRegionCapture(0)
                                        }
                                    }
                                    Item {
                                        Layout.preferredWidth: 188
                                        Layout.preferredHeight: 188
                                        Rectangle {
                                            anchors.centerIn: parent
                                            width: 164
                                            height: 164
                                            radius: 82
                                            color: paletteColors.accent
                                            opacity: 0.12
                                        }
                                        Rectangle {
                                            anchors.centerIn: parent
                                            width: 126
                                            height: 126
                                            radius: 63
                                            color: paletteColors.card
                                            border.color: paletteColors.accent
                                            border.width: 2
                                        }
                                        Rectangle {
                                            anchors.centerIn: parent
                                            width: 52
                                            height: 52
                                            radius: 26
                                            color: paletteColors.accent
                                        }
                                        Rectangle {
                                            anchors.centerIn: parent
                                            width: 12
                                            height: 12
                                            radius: RoTokens.radiusXs
                                            color: RoPalette.selectionText
                                        }
                                    }
                                }
                            }
                            Text {
                                text: qsTr("Choose a capture mode")
                                color: paletteColors.text
                                font.pixelSize: 16
                                font.weight: Font.DemiBold
                                Layout.topMargin: 4
                            }
                            GridLayout {
                                Layout.fillWidth: true
                                columns: width > 660 ? 2 : 1
                                columnSpacing: 12
                                rowSpacing: 12
                                ModeCard {
                                    title: qsTr("Region")
                                    detail: qsTr("Draw a precise selection with the loupe.")
                                    keyText: "⇧ PrtSc"
                                    glyphSource: "qrc:/qt/qml/ro_screenshot/assets/icon-crosshair.svg"
                                    colors: paletteColors
                                    onClicked: captureEngine.requestRegionCapture(0)
                                }
                                ModeCard {
                                    title: qsTr("Entire display")
                                    detail: qsTr("Capture every connected display at once.")
                                    keyText: "PrtSc"
                                    glyphSource: "qrc:/qt/qml/ro_screenshot/assets/icon-fullscreen.svg"
                                    colors: paletteColors
                                    onClicked: captureEngine.requestFullscreenCapture(0)
                                }
                                ModeCard {
                                    title: qsTr("Active window")
                                    detail: qsTr("Keep the current window in focus.")
                                    keyText: "⌥ PrtSc"
                                    glyphSource: "qrc:/qt/qml/ro_screenshot/assets/icon-window.svg"
                                    colors: paletteColors
                                    onClicked: captureEngine.requestWindowCapture(0)
                                }
                                ModeCard {
                                    title: qsTr("Timed region")
                                    detail: qsTr("A five-second pause for menus and popovers.")
                                    keyText: "⌃ PrtSc"
                                    glyphSource: "qrc:/qt/qml/ro_screenshot/assets/icon-timer.svg"
                                    colors: paletteColors
                                    onClicked: captureEngine.requestRegionCapture(5)
                                }
                            }
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 52
                                radius: 13
                                color: paletteColors.shell
                                border.color: paletteColors.border
                                border.width: 1
                                visible: captureEngine.hasLastRegion
                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 15
                                    anchors.rightMargin: 10
                                    Text {
                                        text: qsTr("Repeat the last region")
                                        color: paletteColors.text
                                        font.pixelSize: 13
                                        font.weight: Font.DemiBold
                                        Layout.fillWidth: true
                                    }
                                    Text {
                                        text: "⌃⇧R"
                                        color: paletteColors.textSoft
                                        font.family: "monospace"
                                        font.pixelSize: 11
                                    }
                                    RoButton {
                                        text: qsTr("Repeat")
                                        colors: paletteColors
                                        implicitWidth: 82
                                        onClicked: captureEngine.requestLastRegionCapture(0)
                                    }
                                }
                            }
                        }
                    }
                }
                LibraryView {
                    colors: paletteColors
                }
                SettingsView {
                    colors: paletteColors
                }
            }
        }
    }

    Rectangle {
        parent: Overlay.overlay
        width: Math.min(440, parent.width - 40)
        height: toastRow.implicitHeight + 24
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 26
        radius: 13
        color: paletteColors.cardStrong
        border.color: root.statusIsError ? paletteColors.danger : paletteColors.success
        border.width: 1
        visible: root.statusMessage.length > 0
        opacity: visible ? 1 : 0
        z: 1000
        Behavior on opacity {
            NumberAnimation {
                duration: RoTokens.motionFastMs * RoMotionScale.factor
            }
        }
        RowLayout {
            id: toastRow
            anchors.fill: parent
            anchors.margins: 12
            spacing: 10
            Rectangle {
                Layout.preferredWidth: 8
                Layout.preferredHeight: 8
                radius: 4
                color: root.statusIsError ? paletteColors.danger : paletteColors.success
            }
            Text {
                text: root.statusMessage
                color: paletteColors.text
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
        }
    }

    component NavButton: Button {
        id: navButton
        property string label: ""
        property string glyphSource: ""
        property int badge: 0
        property bool active: false
        property var colors: RoPalette
        Layout.fillWidth: true
        implicitHeight: 42
        hoverEnabled: true
        focusPolicy: Qt.StrongFocus
        Accessible.name: label
        contentItem: RowLayout {
            spacing: 10
            RoIcon {
                color: navButton.active ? RoPalette.selectionText : RoPalette.textMuted
                iconSource: navButton.glyphSource
                sourceSize.width: 17
                sourceSize.height: 17
                Layout.preferredWidth: 17
                Layout.preferredHeight: 17
            }
            Text {
                text: navButton.label
                color: navButton.active ? RoPalette.selectionText : navButton.colors.textMuted
                font.pixelSize: 13
                font.weight: navButton.active ? Font.DemiBold : Font.Normal
                Layout.fillWidth: true
            }
            Rectangle {
                visible: navButton.badge > 0
                Layout.preferredWidth: Math.max(19, badgeText.implicitWidth + 8)
                Layout.preferredHeight: 19
                radius: 9.5
                color: navButton.active ? RoPalette.selectionText : navButton.colors.cardHover
                Text {
                    id: badgeText
                    anchors.centerIn: parent
                    text: navButton.badge
                    color: navButton.active ? navButton.colors.accent : navButton.colors.textSoft
                    font.pixelSize: 10
                    font.weight: Font.DemiBold
                }
            }
        }
        background: Rectangle {
            radius: 10
            color: navButton.active ? navButton.colors.accent : (navButton.hovered ? navButton.colors.cardHover : "transparent")
            border.color: navButton.activeFocus ? navButton.colors.accent : "transparent"
            border.width: navButton.activeFocus ? 2 : 0
        }
    }
    component KeyHint: RowLayout {
        property string title: ""
        property string key: ""
        property var colors: RoPalette
        Layout.fillWidth: true
        Layout.leftMargin: 8
        Layout.rightMargin: 8
        Text {
            text: parent.title
            color: parent.colors.textSoft
            font.pixelSize: 11
            Layout.fillWidth: true
        }
        Text {
            text: parent.key
            color: parent.colors.textMuted
            font.pixelSize: 10
            font.family: "monospace"
        }
    }
    component ModeCard: Button {
        id: modeButton
        property string title: ""
        property string detail: ""
        property string keyText: ""
        property string glyphSource: ""
        property var colors: RoPalette
        Layout.fillWidth: true
        Layout.preferredHeight: 112
        hoverEnabled: true
        focusPolicy: Qt.StrongFocus
        Accessible.name: title
        Accessible.description: detail
        contentItem: RowLayout {
            anchors.fill: parent
            anchors.margins: 15
            spacing: 13
            Rectangle {
                Layout.preferredWidth: 42
                Layout.preferredHeight: 42
                radius: RoTokens.radiusMd
                color: modeButton.colors.accentSoft
                RoIcon {
                    anchors.centerIn: parent
                    iconSource: modeButton.glyphSource
                    sourceSize.width: 21
                    sourceSize.height: 21
                }
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 3
                Text {
                    text: modeButton.title
                    color: modeButton.colors.text
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                }
                Text {
                    text: modeButton.detail
                    color: modeButton.colors.textMuted
                    font.pixelSize: 11
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }
            Text {
                text: modeButton.keyText
                color: modeButton.colors.textSoft
                font.family: "monospace"
                font.pixelSize: 10
                Layout.alignment: Qt.AlignTop
            }
        }
        background: Rectangle {
            radius: 15
            color: modeButton.hovered ? modeButton.colors.cardHover : modeButton.colors.card
            border.color: modeButton.activeFocus || modeButton.hovered ? modeButton.colors.accentMuted : modeButton.colors.border
            border.width: 1
        }
    }
}
