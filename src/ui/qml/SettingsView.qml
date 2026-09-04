import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "components"

Item {
    id: root
    property var colors: ({})
    property bool isSaveDirValid: settingsManager.saveDirectory.length > 0

    FolderDialog {
        id: folderDialog
        title: qsTr("Choose save folder")
        currentFolder: "file://" + settingsManager.saveDirectory
        onAccepted: {
            var path = selectedFolder.toString()
            if (path.startsWith("file://")) path = path.substring(7)
            settingsManager.saveDirectory = path
            snackbar.showMessage(qsTr("Save folder updated."), "success")
        }
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: Math.min(parent.width - 48, 720)
        contentHeight: settingsColumn.implicitHeight + 48
        clip: true
        ColumnLayout {
            id: settingsColumn
            width: Math.min(parent.width - 48, 720)
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 14
            Item { height: 12 }

            RowLayout {
                Layout.fillWidth: true
                Text { text: qsTr("Settings"); color: colors.text; font.pixelSize: 20; font.bold: true; Layout.fillWidth: true }
            }

            RoSection {
                Layout.fillWidth: true
                title: qsTr("Storage")
                iconSource: "qrc:/qt/qml/ro_screenshot/assets/icon-folder.svg"
                colors: root.colors

                RowLayout {
                    Layout.fillWidth: true; spacing: 8
                    TextField {
                        Layout.fillWidth: true
                        text: settingsManager.saveDirectory
                        color: colors.text; font.pixelSize: 13
                        background: Rectangle { implicitHeight: 36; color: colors.codeBg; border.color: colors.border; radius: 8 }
                        onEditingFinished: { settingsManager.saveDirectory = text; snackbar.showMessage(qsTr("Save folder updated."), "success") }
                    }
                    RoButton { text: qsTr("Browse"); variant: "primary"; colors: root.colors; implicitWidth: 92; onClicked: folderDialog.open() }
                }
                RowLayout {
                    Layout.fillWidth: true
                    Text { text: qsTr("Group by month subfolders"); color: colors.textMuted; font.pixelSize: 13; Layout.fillWidth: true }
                    Switch { checked: settingsManager.createSubfoldersByMonth; onToggled: settingsManager.createSubfoldersByMonth = checked }
                }
            }

            RoSection {
                Layout.fillWidth: true
                title: qsTr("File naming & format")
                iconSource: "qrc:/qt/qml/ro_screenshot/assets/icon-tags.svg"
                colors: root.colors

                TextField {
                    Layout.fillWidth: true
                    text: settingsManager.fileNameTemplate
                    color: colors.text; font.pixelSize: 13; font.family: "monospace"
                    background: Rectangle { implicitHeight: 36; color: colors.codeBg; border.color: colors.border; radius: 8 }
                    onEditingFinished: settingsManager.fileNameTemplate = text
                }
                Text { text: qsTr("Preview: %1").arg(settingsManager.previewFileName); color: colors.placeholder; font.pixelSize: 11; font.family: "monospace" }
                RowLayout {
                    Layout.fillWidth: true; spacing: 6
                    Text { text: qsTr("Format"); color: colors.textMuted; font.pixelSize: 12; Layout.preferredWidth: 60 }
                    Repeater {
                        model: ["png", "jpg", "webp"]
                        delegate: Button {
                            text: modelData.toUpperCase()
                            checkable: true; checked: settingsManager.imageFormat === modelData
                            onClicked: settingsManager.imageFormat = modelData
                            contentItem: Text { text: parent.text; color: parent.checked ? "#FFFFFF" : colors.textSoft; font.pixelSize: 11; font.bold: parent.checked; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            background: Rectangle { implicitWidth: 64; implicitHeight: 30; color: parent.checked ? colors.accent : (parent.hovered ? colors.cardHover : colors.codeBg); border.color: parent.checked ? "transparent" : colors.border; radius: 7 }
                        }
                    }
                    Item { Layout.fillWidth: true }
                    Text { visible: settingsManager.imageFormat === "jpg"; text: qsTr("Quality %1%").arg(settingsManager.jpegQuality); color: colors.textSoft; font.pixelSize: 12 }
                    Slider { visible: settingsManager.imageFormat === "jpg"; Layout.preferredWidth: 140; from: 10; to: 100; stepSize: 5; value: settingsManager.jpegQuality; onMoved: settingsManager.jpegQuality = value }
                }
            }

            RoSection {
                Layout.fillWidth: true
                title: qsTr("After capture")
                iconSource: "qrc:/qt/qml/ro_screenshot/assets/icon-bolt.svg"
                colors: root.colors

                InlineStatus {
                    visible: !settingsManager.autoCopyToClipboard && !settingsManager.autoSaveToDisk
                    status: "warning"
                    text: qsTr("Both copy and save are off — captures will be lost.")
                    colors: root.colors
                }
                RowLayout { Layout.fillWidth: true; Text { text: qsTr("Copy to clipboard"); color: colors.textMuted; font.pixelSize: 13; Layout.fillWidth: true } Switch { checked: settingsManager.autoCopyToClipboard; onToggled: settingsManager.autoCopyToClipboard = checked } }
                RowLayout { Layout.fillWidth: true; Text { text: qsTr("Save to disk"); color: colors.textMuted; font.pixelSize: 13; Layout.fillWidth: true } Switch { checked: settingsManager.autoSaveToDisk; onToggled: settingsManager.autoSaveToDisk = checked } }
                RowLayout { Layout.fillWidth: true; Text { text: qsTr("Floating preview card"); color: colors.textMuted; font.pixelSize: 13; Layout.fillWidth: true } Switch { checked: settingsManager.showFloatingThumbnail; onToggled: settingsManager.showFloatingThumbnail = checked } }
                RowLayout { Layout.fillWidth: true; Text { text: qsTr("Desktop notification"); color: colors.textMuted; font.pixelSize: 13; Layout.fillWidth: true } Switch { checked: settingsManager.showNotification; onToggled: settingsManager.showNotification = checked } }
            }

            RoSection {
                Layout.fillWidth: true
                title: qsTr("Selection & loupe")
                iconSource: "qrc:/qt/qml/ro_screenshot/assets/icon-crosshair.svg"
                colors: root.colors

                RowLayout { Layout.fillWidth: true; Text { text: qsTr("Magnifier loupe with color picker"); color: colors.textMuted; font.pixelSize: 13; Layout.fillWidth: true } Switch { checked: settingsManager.magnifierEnabled; onToggled: settingsManager.magnifierEnabled = checked } }
                RowLayout {
                    visible: settingsManager.magnifierEnabled
                    Layout.fillWidth: true; spacing: 10
                    Text { text: qsTr("Zoom %1x").arg(settingsManager.magnifierZoom); color: colors.textSoft; font.pixelSize: 12; Layout.preferredWidth: 70 }
                    Slider { Layout.fillWidth: true; from: 2; to: 16; stepSize: 2; value: settingsManager.magnifierZoom; onMoved: settingsManager.magnifierZoom = value }
                }
                RowLayout { Layout.fillWidth: true; Text { text: qsTr("Close overlay after capture"); color: colors.textMuted; font.pixelSize: 13; Layout.fillWidth: true } Switch { checked: settingsManager.closeOverlayOnCapture; onToggled: settingsManager.closeOverlayOnCapture = checked } }
            }

            RoSection {
                Layout.fillWidth: true
                title: qsTr("Appearance")
                iconSource: "qrc:/qt/qml/ro_screenshot/assets/icon-theme.svg"
                colors: root.colors

                RowLayout {
                    Layout.fillWidth: true; spacing: 8
                    Text { text: qsTr("Theme"); color: colors.textMuted; font.pixelSize: 12; Layout.preferredWidth: 70 }
                    Repeater {
                        model: UiPreferencesManager.availableThemeModes
                        delegate: Button {
                            required property var modelData
                            implicitWidth: 84; implicitHeight: 30
                            text: modelData.label
                            checkable: true; checked: UiPreferencesManager.themeMode === modelData.code
                            onClicked: UiPreferencesManager.setThemeMode(modelData.code)
                            contentItem: Text { text: parent.text; color: parent.checked ? "#FFFFFF" : colors.textSoft; font.pixelSize: 11; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            background: Rectangle { color: parent.checked ? colors.accent : (parent.hovered ? colors.cardHover : colors.codeBg); border.color: parent.checked ? "transparent" : colors.border; radius: 7 }
                        }
                    }
                }
                RowLayout {
                    Layout.fillWidth: true; spacing: 8
                    Text { text: qsTr("Language"); color: colors.textMuted; font.pixelSize: 12; Layout.preferredWidth: 70 }
                    Repeater {
                        model: LanguageManager.availableLanguages
                        delegate: Button {
                            text: modelData.nativeLabel
                            checkable: true; checked: LanguageManager.currentLanguage === modelData.code
                            onClicked: LanguageManager.setCurrentLanguage(modelData.code)
                            contentItem: Text { text: parent.text; color: parent.checked ? "#FFFFFF" : colors.textSoft; font.pixelSize: 11; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            background: Rectangle { implicitWidth: 76; implicitHeight: 30; color: parent.checked ? colors.accent : (parent.hovered ? colors.cardHover : colors.codeBg); border.color: parent.checked ? "transparent" : colors.border; radius: 7 }
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true; spacing: 8
                RoButton { text: qsTr("Clear thumbnail cache"); colors: root.colors; Layout.fillWidth: true; onClicked: if (libraryManager.clearThumbnailCache()) snackbar.showMessage(qsTr("Thumbnail cache cleared."), "success") }
                RoButton {
                    text: qsTr("Reset to defaults"); variant: "dangerGhost"; colors: root.colors; Layout.fillWidth: true
                    onClicked: resetConfirmDialog.open(qsTr("Reset settings"), qsTr("Return all settings to factory defaults?"), true, function() { settingsManager.resetToDefaults(); snackbar.showMessage(qsTr("Settings reset."), "info") })
                }
            }
            Item { height: 16 }
        }
    }

    ConfirmDialog { id: resetConfirmDialog; colors: root.colors }
    AppSnackbar { id: snackbar; colors: root.colors }
}
