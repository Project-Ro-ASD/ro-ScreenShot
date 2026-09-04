import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "components"
import "editor"

Item {
    id: root

    property var colors: ({})
    property int selectedIndex: -1
    property var selectedItem: selectedIndex >= 0 ? libraryManager.getItem(selectedIndex) : null
    property bool isRenameOpen: false
    property string newFileNameInput: ""

    focus: true
    Keys.onDeletePressed: {
        if (libraryManager.selectedCount > 0) trashSelectedItems()
        else if (root.selectedIndex >= 0) trashSingleItem(root.selectedIndex)
    }
    Keys.onReturnPressed: { if (root.selectedIndex >= 0) libraryManager.openFile(root.selectedIndex) }

    function trashSingleItem(idx) {
        var item = libraryManager.getItem(idx)
        if (item && item.filePath) {
            var fileName = item.fileName
            if (libraryManager.deleteItem(idx)) {
                root.selectedIndex = -1
                snackbar.showMessage(qsTr("%1 moved to trash.").arg(fileName), "info", qsTr("Undo"), function() { libraryManager.undoLastTrash() })
            }
        }
    }
    function trashSelectedItems() {
        var count = libraryManager.selectedCount
        if (libraryManager.trashSelected()) {
            root.selectedIndex = -1
            snackbar.showMessage(qsTr("%1 items moved to trash.").arg(count), "info", qsTr("Undo"), function() { libraryManager.undoLastTrash() })
        }
    }
    function requestPermanentDelete(idx) {
        var item = libraryManager.getItem(idx)
        var name = item ? item.fileName : ""
        confirmDialog.open(qsTr("Delete permanently"), qsTr("\"%1\" will be permanently deleted. This cannot be undone.").arg(name), true, function() {
            if (libraryManager.permanentDeleteItem(idx)) { root.selectedIndex = -1; snackbar.showMessage(qsTr("File permanently deleted."), "success") }
        })
    }
    function requestPermanentDeleteSelected() {
        var count = libraryManager.selectedCount
        confirmDialog.open(qsTr("Delete permanently"), qsTr("%1 selected items will be permanently deleted.").arg(count), true, function() {
            if (libraryManager.permanentDeleteSelected()) { root.selectedIndex = -1; snackbar.showMessage(qsTr("Selected items deleted."), "success") }
        })
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // ── Toolbar ──
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 60
                color: colors.card
                Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: colors.border }
                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 16; anchors.rightMargin: 16
                    spacing: 10

                    Rectangle {
                        Layout.preferredWidth: 220; Layout.preferredHeight: 34
                        color: colors.codeBg; border.color: searchField.activeFocus ? colors.accent : colors.border; border.width: 1; radius: 8
                        RowLayout {
                            anchors.fill: parent; anchors.leftMargin: 10; anchors.rightMargin: 8; spacing: 6
                            Image { source: "qrc:/qt/qml/ro_screenshot/assets/icon-search.svg"; sourceSize.width: 13; sourceSize.height: 13; Layout.preferredWidth: 13; Layout.preferredHeight: 13; opacity: 0.6 }
                            TextField {
                                id: searchField
                                Layout.fillWidth: true
                                placeholderText: qsTr("Search screenshots...")
                                placeholderTextColor: colors.placeholder
                                color: colors.text; font.pixelSize: 13
                                background: Item {}
                                onTextChanged: libraryManager.searchQuery = text
                            }
                            Text {
                                visible: searchField.text.length > 0
                                text: "✕"; color: colors.textSoft; font.pixelSize: 12
                                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: searchField.text = "" }
                            }
                        }
                    }

                    RowLayout {
                        spacing: 4
                        Repeater {
                            model: [
                                { label: qsTr("All"), filter: 0 },
                                { label: qsTr("Today"), filter: 1 },
                                { label: qsTr("Week"), filter: 3 },
                                { label: qsTr("Month"), filter: 5 }
                            ]
                            delegate: Button {
                                text: modelData.label
                                checkable: true
                                checked: libraryManager.dateFilter === modelData.filter
                                onClicked: libraryManager.dateFilter = modelData.filter
                                contentItem: Text { text: parent.text; color: parent.checked ? "#FFFFFF" : colors.textSoft; font.pixelSize: 11; font.bold: parent.checked; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                background: Rectangle { implicitWidth: 62; implicitHeight: 30; color: parent.checked ? colors.accent : (parent.hovered ? colors.cardHover : "transparent"); radius: 7 }
                            }
                        }
                    }

                    ComboBox {
                        id: sortCombo
                        model: [ qsTr("Newest"), qsTr("Oldest"), qsTr("Name A–Z"), qsTr("Name Z–A"), qsTr("Largest"), qsTr("Smallest") ]
                        currentIndex: Math.min(libraryManager.sortOrder, 5)
                        onActivated: libraryManager.sortOrder = index
                        implicitWidth: 118; implicitHeight: 30
                        contentItem: Text { text: sortCombo.displayText; color: colors.textMuted; font.pixelSize: 11; verticalAlignment: Text.AlignVCenter; leftPadding: 10; elide: Text.ElideRight }
                        background: Rectangle { color: colors.codeBg; border.color: colors.border; radius: 7 }
                    }

                    Item { Layout.fillWidth: true }

                    Text { text: qsTr("%1 items · %2").arg(libraryManager.count).arg(libraryManager.totalStorageSize); color: colors.placeholder; font.pixelSize: 11 }

                    AccessibleIconButton {
                        iconSource: "assets/icon-refresh.svg"; iconSize: 15; colors: root.colors
                        tooltipText: qsTr("Refresh"); onClicked: libraryManager.refresh()
                    }
                }
            }

            // ── Selection bar ──
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: libraryManager.selectedCount > 0 ? 38 : 0
                visible: libraryManager.selectedCount > 0
                clip: true
                color: colors.accentSoft
                Behavior on Layout.preferredHeight { NumberAnimation { duration: 140 } }
                RowLayout {
                    anchors.fill: parent; anchors.leftMargin: 16; anchors.rightMargin: 16; spacing: 10
                    Text { text: qsTr("%1 selected").arg(libraryManager.selectedCount); color: colors.accent; font.pixelSize: 12; font.bold: true }
                    Text { text: qsTr("Select all"); color: colors.textMuted; font.pixelSize: 12; MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: libraryManager.selectAll() } }
                    Text { text: qsTr("Clear"); color: colors.textMuted; font.pixelSize: 12; MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: libraryManager.clearSelection() } }
                    Item { Layout.fillWidth: true }
                    RoButton { text: qsTr("Move to trash"); variant: "danger"; colors: root.colors; implicitWidth: 120; implicitHeight: 28; onClicked: root.trashSelectedItems() }
                }
            }

            // ── Grid ──
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                RowLayout {
                    anchors.top: parent.top; anchors.horizontalCenter: parent.horizontalCenter; anchors.topMargin: 10
                    visible: libraryManager.isScanning; spacing: 8; z: 10
                    BusyIndicator { implicitWidth: 16; implicitHeight: 16; running: libraryManager.isScanning }
                    Text { text: qsTr("Scanning..."); color: colors.accent; font.pixelSize: 12 }
                }

                ColumnLayout {
                    anchors.centerIn: parent; spacing: 8
                    visible: libraryManager.count === 0 && !libraryManager.isScanning
                    width: 320
                    Image { source: "qrc:/qt/qml/ro_screenshot/assets/icon-gallery-empty.svg"; sourceSize.width: 44; sourceSize.height: 44; Layout.alignment: Qt.AlignHCenter; opacity: 0.6 }
                    Text { text: libraryManager.searchQuery.length > 0 ? qsTr("No results for \"%1\"").arg(libraryManager.searchQuery) : qsTr("No screenshots yet"); color: colors.text; font.pixelSize: 15; font.bold: true; Layout.alignment: Qt.AlignHCenter }
                    Text { text: qsTr("Use Region or Fullscreen capture to get started."); color: colors.placeholder; font.pixelSize: 12; Layout.alignment: Qt.AlignHCenter; horizontalAlignment: Text.AlignHCenter; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    RoButton { visible: libraryManager.searchQuery.length > 0; text: qsTr("Clear search"); colors: root.colors; Layout.alignment: Qt.AlignHCenter; onClicked: searchField.text = "" }
                }

                GridView {
                    id: grid
                    anchors.fill: parent; anchors.margins: 16
                    cellWidth: 224; cellHeight: 196; clip: true
                    visible: libraryManager.count > 0
                    model: libraryManager
                    delegate: Rectangle {
                        width: 210; height: 182
                        color: model.isSelected ? colors.selected : (root.selectedIndex === index ? colors.cardHover : colors.card)
                        border.color: model.isSelected || root.selectedIndex === index ? colors.accent : (itemMouse.containsMouse ? colors.accentMuted : colors.border)
                        border.width: (model.isSelected || root.selectedIndex === index) ? 1.5 : 1
                        radius: 10
                        MouseArea {
                            id: itemMouse
                            anchors.fill: parent; hoverEnabled: true
                            onClicked: function(mouse) {
                                if (mouse.modifiers & Qt.ControlModifier) libraryManager.toggleSelection(index)
                                else root.selectedIndex = index
                            }
                            onDoubleClicked: annotationEditor.open(model.filePath)
                        }
                        ColumnLayout {
                            anchors.fill: parent; anchors.margins: 8; spacing: 6
                            Rectangle {
                                Layout.fillWidth: true; Layout.fillHeight: true
                                color: colors.codeBg; radius: 6; clip: true
                                Image { anchors.fill: parent; source: model.thumbnailUrl; fillMode: Image.PreserveAspectFit; asynchronous: true; cache: true }
                                Rectangle {
                                    anchors.top: parent.top; anchors.right: parent.right; anchors.margins: 6
                                    width: 22; height: 22; radius: 11
                                    color: model.isSelected ? colors.accent : (itemMouse.containsMouse ? colors.card : "transparent")
                                    border.color: model.isSelected ? "transparent" : colors.border
                                    border.width: 1
                                    visible: itemMouse.containsMouse || model.isSelected
                                    Text { anchors.centerIn: parent; text: "✓"; color: model.isSelected ? "#FFFFFF" : colors.textSoft; font.pixelSize: 11; font.bold: true }
                                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: libraryManager.toggleSelection(index) }
                                }
                                Rectangle {
                                    anchors.bottom: parent.bottom; anchors.right: parent.right; anchors.margins: 6
                                    implicitWidth: formatText.implicitWidth + 10; height: 17; radius: 4
                                    color: "#111827"; opacity: 0.8
                                    Text { id: formatText; anchors.centerIn: parent; text: model.format; color: "#FFFFFF"; font.pixelSize: 9; font.bold: true; font.family: "monospace" }
                                }
                            }
                            Text { text: model.fileName; color: colors.text; font.pixelSize: 11; font.bold: true; elide: Text.ElideMiddle; Layout.fillWidth: true }
                            RowLayout {
                                Layout.fillWidth: true
                                Text { text: model.formattedDate; color: colors.placeholder; font.pixelSize: 10; Layout.fillWidth: true; elide: Text.ElideRight }
                                Text { text: model.formattedSize; color: colors.textSoft; font.pixelSize: 10; font.family: "monospace" }
                            }
                        }
                    }
                }
            }
        }

        // ── Detail panel ──
        Rectangle {
            Layout.preferredWidth: (root.selectedItem && libraryManager.selectedCount <= 1) ? 296 : 0
            Layout.fillHeight: true
            color: colors.card
            visible: (root.selectedItem && libraryManager.selectedCount <= 1) !== null
            clip: true
            Behavior on Layout.preferredWidth { NumberAnimation { duration: 140 } }
            Rectangle { anchors.left: parent.left; width: 1; height: parent.height; color: colors.border }

            ColumnLayout {
                anchors.fill: parent; anchors.margins: 16; spacing: 12
                visible: root.selectedItem !== null
                RowLayout {
                    Layout.fillWidth: true
                    Text { text: qsTr("Details"); color: colors.text; font.pixelSize: 14; font.bold: true; Layout.fillWidth: true }
                    Text { text: "✕"; color: colors.textSoft; font.pixelSize: 14; MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: root.selectedIndex = -1 } }
                }
                Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 150
                    color: colors.codeBg; radius: 8; border.color: colors.border; border.width: 1; clip: true
                    Image { anchors.fill: parent; source: root.selectedItem ? root.selectedItem.thumbnailUrl : ""; fillMode: Image.PreserveAspectFit; asynchronous: true }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onDoubleClicked: if (root.selectedItem) annotationEditor.open(root.selectedItem.filePath) }
                }
                Text { text: root.selectedItem ? root.selectedItem.fileName : ""; color: colors.text; font.pixelSize: 12; font.bold: true; wrapMode: Text.WrapAnywhere; Layout.fillWidth: true }
                GridLayout {
                    Layout.fillWidth: true; columns: 2; rowSpacing: 8; columnSpacing: 12
                    Text { text: qsTr("Size"); color: colors.placeholder; font.pixelSize: 11 }
                    Text { text: root.selectedItem ? root.selectedItem.formattedSize : ""; color: colors.textMuted; font.pixelSize: 11; Layout.alignment: Qt.AlignRight }
                    Text { text: qsTr("Resolution"); color: colors.placeholder; font.pixelSize: 11 }
                    Text { text: root.selectedItem ? root.selectedItem.resolution : ""; color: colors.textMuted; font.pixelSize: 11; Layout.alignment: Qt.AlignRight }
                    Text { text: qsTr("Date"); color: colors.placeholder; font.pixelSize: 11 }
                    Text { text: root.selectedItem ? root.selectedItem.formattedDate : ""; color: colors.textMuted; font.pixelSize: 11; Layout.alignment: Qt.AlignRight }
                }
                Item { Layout.fillHeight: true }
                ColumnLayout {
                    Layout.fillWidth: true; spacing: 8
                    RoButton { Layout.fillWidth: true; text: qsTr("Edit & Annotate"); variant: "primary"; colors: root.colors; onClicked: if (root.selectedItem) annotationEditor.open(root.selectedItem.filePath) }
                    RoButton { Layout.fillWidth: true; text: qsTr("Copy to clipboard"); colors: root.colors; onClicked: if (root.selectedIndex >= 0) { libraryManager.copyToClipboard(root.selectedIndex); snackbar.showMessage(qsTr("Copied to clipboard."), "success") } }
                    RowLayout {
                        Layout.fillWidth: true; spacing: 8
                        RoButton { Layout.fillWidth: true; text: qsTr("Show in folder"); colors: root.colors; onClicked: if (root.selectedIndex >= 0) libraryManager.openInFolder(root.selectedIndex) }
                        RoButton { Layout.fillWidth: true; text: qsTr("Rename"); colors: root.colors; onClicked: if (root.selectedItem) { root.newFileNameInput = root.selectedItem.fileName; root.isRenameOpen = true } }
                    }
                    RoButton { Layout.fillWidth: true; text: qsTr("Move to trash"); variant: "dangerGhost"; colors: root.colors; onClicked: if (root.selectedIndex >= 0) root.trashSingleItem(root.selectedIndex) }
                    Text {
                        text: qsTr("Delete permanently...")
                        color: colors.placeholder; font.pixelSize: 11; Layout.alignment: Qt.AlignHCenter
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: if (root.selectedIndex >= 0) root.requestPermanentDelete(root.selectedIndex) }
                    }
                }
            }
        }
    }

    Rectangle {
        anchors.fill: parent; color: "#000000"; opacity: root.isRenameOpen ? 0.5 : 0.0; visible: opacity > 0.0
        Behavior on opacity { NumberAnimation { duration: 140 } }
        MouseArea { anchors.fill: parent; onClicked: root.isRenameOpen = false }
    }
    Rectangle {
        width: 360; implicitHeight: renameCol.implicitHeight + 40
        anchors.centerIn: parent; radius: 12; color: colors.card; border.color: colors.border; border.width: 1
        visible: root.isRenameOpen; z: 500
        ColumnLayout {
            id: renameCol; anchors.fill: parent; anchors.margins: 20; spacing: 12
            Text { text: qsTr("Rename file"); color: colors.text; font.pixelSize: 14; font.bold: true }
            TextField {
                id: renameField; Layout.fillWidth: true; text: root.newFileNameInput; color: colors.text; font.pixelSize: 13
                background: Rectangle { implicitHeight: 36; color: colors.codeBg; border.color: colors.border; radius: 8 }
                onAccepted: {
                    var res = libraryManager.renameItem(root.selectedIndex, text)
                    if (res.success) { root.isRenameOpen = false; snackbar.showMessage(qsTr("File renamed."), "success") }
                    else snackbar.showMessage(res.error || qsTr("Rename failed."), "error")
                }
            }
            RowLayout {
                Layout.fillWidth: true; spacing: 8
                Item { Layout.fillWidth: true }
                RoButton { text: qsTr("Cancel"); variant: "ghost"; colors: root.colors; implicitWidth: 84; onClicked: root.isRenameOpen = false }
                RoButton {
                    text: qsTr("Save"); variant: "primary"; colors: root.colors; implicitWidth: 84
                    onClicked: {
                        var res = libraryManager.renameItem(root.selectedIndex, renameField.text)
                        if (res.success) { root.isRenameOpen = false; snackbar.showMessage(qsTr("File renamed."), "success") }
                        else snackbar.showMessage(res.error || qsTr("Rename failed."), "error")
                    }
                }
            }
        }
    }

    AnnotationEditor {
        id: annotationEditor; colors: root.colors
        onSaved: function(path) { libraryManager.refresh(); snackbar.showMessage(qsTr("Annotation saved."), "success") }
    }
    ConfirmDialog { id: confirmDialog; colors: root.colors }
    AppSnackbar { id: snackbar; colors: root.colors }
}
