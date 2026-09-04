import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import "components"
import "editor"

Item {
    id: root

    property var colors: ({})
    property int selectedIndex: -1
    property var selectedItem: selectedIndex >= 0 ? libraryManager.getItem(selectedIndex) : null
    property bool isRenameOpen: false
    property string newFileNameInput: ""

    // Keyboard Shortcuts
    focus: true
    Keys.onDeletePressed: {
        if (libraryManager.selectedCount > 0) {
            trashSelectedItems()
        } else if (root.selectedIndex >= 0) {
            trashSingleItem(root.selectedIndex)
        }
    }
    Keys.onReturnPressed: {
        if (root.selectedIndex >= 0) {
            libraryManager.openFile(root.selectedIndex)
        }
    }

    function trashSingleItem(idx) {
        var item = libraryManager.getItem(idx)
        if (item && item.filePath) {
            var fileName = item.fileName
            if (libraryManager.deleteItem(idx)) {
                root.selectedIndex = -1
                snackbar.showMessage(
                    qsTr("%1 çöp kutusuna taşındı.").arg(fileName),
                    "info",
                    qsTr("Geri Al"),
                    function() {
                        libraryManager.undoLastTrash()
                    }
                )
            }
        }
    }

    function trashSelectedItems() {
        var count = libraryManager.selectedCount
        if (libraryManager.trashSelected()) {
            root.selectedIndex = -1
            snackbar.showMessage(
                qsTr("%1 öğe çöp kutusuna taşındı.").arg(count),
                "info",
                qsTr("Geri Al"),
                function() {
                    libraryManager.undoLastTrash()
                }
            )
        }
    }

    function requestPermanentDelete(idx) {
        var item = libraryManager.getItem(idx)
        var name = item ? item.fileName : ""
        confirmDialog.open(
            qsTr("Kalıcı Olarak Sil"),
            qsTr("\"%1\" dosyasını kalıcı olarak silmek istediğinizden emin misiniz? Bu işlem geri alınamaz.").arg(name),
            true,
            function() {
                if (libraryManager.permanentDeleteItem(idx)) {
                    root.selectedIndex = -1
                    snackbar.showMessage(qsTr("Dosya kalıcı olarak silindi."), "success")
                }
            }
        )
    }

    function requestPermanentDeleteSelected() {
        var count = libraryManager.selectedCount
        confirmDialog.open(
            qsTr("Kalıcı Olarak Sil"),
            qsTr("Seçili %1 öğeyi kalıcı olarak silmek istediğinizden emin misiniz? Bu işlem geri alınamaz.").arg(count),
            true,
            function() {
                if (libraryManager.permanentDeleteSelected()) {
                    root.selectedIndex = -1
                    snackbar.showMessage(qsTr("Seçili öğeler kalıcı olarak silindi."), "success")
                }
            }
        )
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Left / Main Content: Search, Filter, Grid
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: parent.height
            spacing: 12

            // Filter & Search Toolbar
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 64
                color: colors.shell
                border.color: colors.border
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    spacing: 12

                    // Search input
                    Rectangle {
                        Layout.preferredWidth: 230
                        Layout.preferredHeight: 38
                        color: colors.cardStrong
                        border.color: searchField.activeFocus ? colors.accentB : colors.border
                        border.width: 1
                        radius: 6

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10
                            spacing: 8

                            Image {
                                source: "qrc:/qt/qml/ro_screenshot/assets/icon-search.svg"
                                sourceSize.width: 13
                                sourceSize.height: 13
                                Layout.preferredWidth: 13
                                Layout.preferredHeight: 13
                            }

                            TextField {
                                id: searchField
                                Layout.fillWidth: true
                                placeholderText: qsTr("Görsellerde ara...")
                                placeholderTextColor: colors.placeholder
                                color: colors.text
                                font.pixelSize: 13
                                background: Item {}
                                onTextChanged: libraryManager.searchQuery = text
                                Accessible.name: qsTr("Görsel arama kutusu")
                            }

                            Button {
                                visible: searchField.text.length > 0
                                flat: true
                                onClicked: searchField.text = ""
                                contentItem: Text {
                                    text: "✕"
                                    color: colors.textSoft
                                    font.pixelSize: 12
                                }
                                background: Item {}
                            }
                        }
                    }

                    // Date Filters
                    RowLayout {
                        spacing: 4
                        Repeater {
                            model: [
                                { label: qsTr("Tümü"), filter: 0 },
                                { label: qsTr("Bugün"), filter: 1 },
                                { label: qsTr("Dün"), filter: 2 },
                                { label: qsTr("Bu Hafta"), filter: 3 },
                                { label: qsTr("Son 7 Gün"), filter: 4 },
                                { label: qsTr("Bu Ay"), filter: 5 }
                            ]
                            delegate: Button {
                                text: modelData.label
                                checkable: true
                                checked: libraryManager.dateFilter === modelData.filter
                                onClicked: libraryManager.dateFilter = modelData.filter
                                contentItem: Text {
                                    text: parent.text
                                    color: parent.checked ? "#FFFFFF" : colors.textSoft
                                    font.pixelSize: 11
                                    font.bold: parent.checked
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                background: Rectangle {
                                    implicitWidth: 64
                                    implicitHeight: 32
                                    color: parent.checked ? colors.accentB : (parent.hovered ? colors.border : colors.cardStrong)
                                    radius: 6
                                }
                            }
                        }
                    }

                    // Sort Order ComboBox
                    ComboBox {
                        id: sortCombo
                        model: [
                            qsTr("Yeniden Eskiye"),
                            qsTr("Eskiden Yeniye"),
                            qsTr("İsim (A-Z)"),
                            qsTr("İsim (Z-A)"),
                            qsTr("Boyut (Büyük)"),
                            qsTr("Boyut (Küçük)"),
                            qsTr("Çözünürlük")
                        ]
                        currentIndex: libraryManager.sortOrder
                        onActivated: libraryManager.sortOrder = index
                        implicitWidth: 130
                        implicitHeight: 32

                        contentItem: Text {
                            text: sortCombo.displayText
                            color: colors.text
                            font.pixelSize: 11
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 8
                        }
                        background: Rectangle {
                            color: colors.cardStrong
                            border.color: colors.border
                            radius: 6
                        }
                    }

                    Item { Layout.fillWidth: true }

                    // Storage Badge
                    Rectangle {
                        height: 28
                        implicitWidth: storageText.implicitWidth + 22
                        radius: 6
                        color: colors.cardStrong
                        border.color: colors.border
                        border.width: 1

                        RowLayout {
                            anchors.centerIn: parent
                            spacing: 6

                            Image {
                                source: "qrc:/qt/qml/ro_screenshot/assets/icon-folder.svg"
                                sourceSize.width: 13
                                sourceSize.height: 13
                                Layout.preferredWidth: 13
                                Layout.preferredHeight: 13
                            }

                            Text {
                                id: storageText
                                text: qsTr("%1 Dahilinde").arg(libraryManager.totalStorageSize)
                                color: colors.textSoft
                                font.pixelSize: 11
                                Layout.alignment: Qt.AlignVCenter
                            }
                        }
                    }

                    // Refresh Button with Scanning Animation
                    AccessibleIconButton {
                        iconSource: "assets/icon-refresh.svg"
                        iconSize: 16
                        colors: root.colors
                        tooltipText: qsTr("Yenile")
                        onClicked: libraryManager.refresh()
                    }
                }
            }

            // Active Filters & Multi-selection Bar
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 32
                color: colors.shellAlt
                border.color: colors.border
                border.width: 1
                visible: libraryManager.selectedCount > 0 || libraryManager.searchQuery.length > 0 || libraryManager.dateFilter > 0

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    spacing: 12

                    Text {
                        text: qsTr("%1 görsel listeleniyor").arg(libraryManager.count)
                        color: colors.textSoft
                        font.pixelSize: 11
                    }

                    Rectangle {
                        width: 1
                        height: 16
                        color: colors.border
                    }

                    // Multi-selection actions
                    RowLayout {
                        visible: libraryManager.selectedCount > 0
                        spacing: 8

                        Text {
                            text: qsTr("%1 öğe seçildi").arg(libraryManager.selectedCount)
                            color: colors.accentB
                            font.pixelSize: 11
                            font.bold: true
                        }

                        Button {
                            text: qsTr("Tümünü Seç")
                            flat: true
                            onClicked: libraryManager.selectAll()
                            contentItem: Text { text: parent.text; color: colors.textSoft; font.pixelSize: 11 }
                        }

                        Button {
                            text: qsTr("Seçimi Temizle")
                            flat: true
                            onClicked: libraryManager.clearSelection()
                            contentItem: Text { text: parent.text; color: colors.textSoft; font.pixelSize: 11 }
                        }

                        Button {
                            text: qsTr("Seçilenleri Çöpe At")
                            onClicked: root.trashSelectedItems()
                            contentItem: Text { text: parent.text; color: "#FFFFFF"; font.pixelSize: 11; font.bold: true }
                            background: Rectangle { implicitHeight: 24; implicitWidth: 120; radius: 4; color: colors.danger }
                        }
                    }

                    Item { Layout.fillWidth: true }
                }
            }

            // Grid or Empty State
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                // Scanning Progress / Skeleton Indicator
                RowLayout {
                    anchors.top: parent.top
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.topMargin: 8
                    visible: libraryManager.isScanning
                    spacing: 8
                    z: 10

                    BusyIndicator {
                        implicitWidth: 18
                        implicitHeight: 18
                        running: libraryManager.isScanning
                    }
                    Text {
                        text: qsTr("Görseller taranıyor...")
                        color: colors.accentB
                        font.pixelSize: 12
                    }
                }

                // Empty State 1: Search no results
                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 12
                    visible: libraryManager.count === 0 && libraryManager.searchQuery.length > 0 && !libraryManager.isScanning

                    Text {
                        text: qsTr("Arama sonucu bulunamadı: \"%1\"").arg(libraryManager.searchQuery)
                        color: colors.text
                        font.pixelSize: 16
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Button {
                        text: qsTr("Aramayı Temizle")
                        Layout.alignment: Qt.AlignHCenter
                        onClicked: searchField.text = ""
                    }
                }

                // Empty State 2: No screenshots found in folder
                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 12
                    visible: libraryManager.count === 0 && libraryManager.searchQuery.length === 0 && !libraryManager.isScanning

                    Image {
                        source: "qrc:/qt/qml/ro_screenshot/assets/icon-gallery-empty.svg"
                        sourceSize.width: 48
                        sourceSize.height: 48
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Text {
                        text: qsTr("Henüz ekran görüntüsü bulunamadı.")
                        color: colors.text
                        font.pixelSize: 16
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Text {
                        text: qsTr("Kayıt klasörü: %1").arg(settingsManager.saveDirectory)
                        color: colors.placeholder
                        font.pixelSize: 12
                        Layout.alignment: Qt.AlignHCenter
                    }
                }

                // Grid View
                GridView {
                    id: grid
                    anchors.fill: parent
                    anchors.margins: 16
                    cellWidth: 230
                    cellHeight: 200
                    clip: true
                    visible: libraryManager.count > 0
                    model: libraryManager

                    delegate: Rectangle {
                        id: itemRect
                        width: 216
                        height: 186
                        color: model.isSelected ? colors.selected : (root.selectedIndex === index ? colors.cardStrong : (itemMouse.containsMouse ? colors.cardStrong : colors.card))
                        border.color: model.isSelected ? colors.accentA : (root.selectedIndex === index ? colors.accentB : (itemMouse.containsMouse ? colors.muted : colors.border))
                        border.width: (model.isSelected || root.selectedIndex === index) ? 2 : 1
                        radius: 8

                        MouseArea {
                            id: itemMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: function(mouse) {
                                if (mouse.modifiers & Qt.ControlModifier) {
                                    libraryManager.toggleSelection(index)
                                } else {
                                    root.selectedIndex = index
                                }
                            }
                            onDoubleClicked: {
                                annotationEditor.open(model.filePath)
                            }
                        }

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 6

                            // Thumbnail Image Container
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                color: colors.shellAlt
                                radius: 4
                                clip: true

                                Image {
                                    anchors.fill: parent
                                    source: model.thumbnailUrl
                                    fillMode: Image.PreserveAspectFit
                                    asynchronous: true
                                    cache: true
                                }

                                // Selection checkbox
                                CheckBox {
                                    anchors.top: parent.top
                                    anchors.right: parent.right
                                    anchors.margins: 4
                                    checked: model.isSelected
                                    onToggled: libraryManager.toggleSelection(index)
                                    visible: itemMouse.containsMouse || model.isSelected
                                }

                                // Format badge
                                Rectangle {
                                    anchors.bottom: parent.bottom
                                    anchors.right: parent.right
                                    anchors.margins: 4
                                    width: formatText.implicitWidth + 8
                                    height: 16
                                    radius: 3
                                    color: "#000000"
                                    opacity: 0.75
                                    Text {
                                        id: formatText
                                        anchors.centerIn: parent
                                        text: model.format
                                        color: "#FFFFFF"
                                        font.pixelSize: 9
                                        font.bold: true
                                    }
                                }
                            }

                            // Meta Row
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2

                                Text {
                                    text: model.fileName
                                    color: colors.text
                                    font.pixelSize: 11
                                    font.bold: true
                                    elide: Text.ElideMiddle
                                    Layout.fillWidth: true
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    Text {
                                        text: model.formattedDate
                                        color: colors.placeholder
                                        font.pixelSize: 10
                                    }
                                    Item { Layout.fillWidth: true }
                                    Text {
                                        text: model.formattedSize
                                        color: colors.textSoft
                                        font.pixelSize: 10
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Right Inspection Panel (Selected Item Details)
        Rectangle {
            Layout.preferredWidth: (root.selectedItem && libraryManager.selectedCount <= 1) ? 310 : 0
            Layout.fillHeight: true
            color: colors.shell
            border.color: colors.border
            border.width: 1
            visible: (root.selectedItem && libraryManager.selectedCount <= 1) !== null
            clip: true

            Behavior on Layout.preferredWidth {
                NumberAnimation { duration: 150 }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 14
                visible: root.selectedItem !== null

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: qsTr("Görsel Detayları")
                        color: colors.text
                        font.pixelSize: 15
                        font.bold: true
                    }
                    Item { Layout.fillWidth: true }
                    Button {
                        flat: true
                        onClicked: root.selectedIndex = -1
                        contentItem: Text {
                            text: "✕"
                            color: colors.textSoft
                            font.pixelSize: 14
                        }
                        background: Item {}
                    }
                }

                // Large Preview Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 160
                    color: colors.shellAlt
                    radius: 8
                    border.color: colors.border
                    border.width: 1
                    clip: true

                    Image {
                        anchors.fill: parent
                        source: root.selectedItem ? root.selectedItem.thumbnailUrl : ""
                        fillMode: Image.PreserveAspectFit
                        asynchronous: true
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onDoubleClicked: {
                            if (root.selectedItem) {
                                annotationEditor.open(root.selectedItem.filePath)
                            }
                        }
                    }
                }

                // Info Rows
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    ColumnLayout {
                        spacing: 2
                        Text {
                            text: qsTr("Dosya Adı")
                            color: colors.placeholder
                            font.pixelSize: 11
                        }
                        Text {
                            text: root.selectedItem ? root.selectedItem.fileName : ""
                            color: colors.text
                            font.pixelSize: 12
                            font.bold: true
                            wrapMode: Text.WrapAnywhere
                            Layout.fillWidth: true
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Text {
                                text: qsTr("Çözünürlük")
                                color: colors.placeholder
                                font.pixelSize: 11
                            }
                            Text {
                                text: root.selectedItem ? root.selectedItem.resolution : ""
                                color: colors.textMuted
                                font.pixelSize: 12
                            }
                        }
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Text {
                                text: qsTr("Boyut")
                                color: colors.placeholder
                                font.pixelSize: 11
                            }
                            Text {
                                text: root.selectedItem ? root.selectedItem.formattedSize : ""
                                color: colors.textMuted
                                font.pixelSize: 12
                            }
                        }
                    }

                    ColumnLayout {
                        spacing: 2
                        Text {
                            text: qsTr("Oluşturulma Tarihi")
                            color: colors.placeholder
                            font.pixelSize: 11
                        }
                        Text {
                            text: root.selectedItem ? root.selectedItem.formattedDate : ""
                            color: colors.textMuted
                            font.pixelSize: 12
                        }
                    }
                }

                Item { Layout.fillHeight: true }

                // Actions
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    // Edit / Annotate Button
                    Button {
                        Layout.fillWidth: true
                        text: qsTr("Düzenle & Not Ekle")
                        onClicked: {
                            if (root.selectedItem) {
                                annotationEditor.open(root.selectedItem.filePath)
                            }
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "#FFFFFF"
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            implicitHeight: 34
                            color: parent.hovered ? colors.accentC : colors.accentA
                            radius: 6
                        }
                    }

                    // Mockup Frame Generator Button
                    Button {
                        Layout.fillWidth: true
                        text: qsTr("🖼️ Mockup Çerçevesi Ekle")
                        onClicked: {
                            if (root.selectedIndex >= 0 && root.Window.window && root.Window.window.openMockupDialog) {
                                root.Window.window.openMockupDialog(root.selectedIndex)
                            }
                        }
                        contentItem: Text {
                            text: parent.text
                            color: colors.text
                            font.pixelSize: 11
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            implicitHeight: 32
                            color: parent.hovered ? colors.border : colors.cardStrong
                            border.color: colors.border
                            radius: 6
                        }
                    }

                    // DevKit & Palette Button
                    Button {
                        Layout.fillWidth: true
                        text: qsTr("🎨 Renk & Tablo Kiti")
                        onClicked: {
                            if (root.selectedIndex >= 0 && root.Window.window && root.Window.window.openDevKitDialog) {
                                root.Window.window.openDevKitDialog(root.selectedIndex)
                            }
                        }
                        contentItem: Text {
                            text: parent.text
                            color: colors.text
                            font.pixelSize: 11
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            implicitHeight: 32
                            color: parent.hovered ? colors.border : colors.cardStrong
                            border.color: colors.border
                            radius: 6
                        }
                    }

                    // Copy to Clipboard
                    Button {
                        Layout.fillWidth: true
                        text: qsTr("Panoya Kopyala")
                        onClicked: {
                            if (root.selectedIndex >= 0) {
                                libraryManager.copyToClipboard(root.selectedIndex)
                                snackbar.showMessage(qsTr("Görsel panoya kopyalandı."), "success")
                            }
                        }
                        contentItem: Text {
                            text: parent.text
                            color: colors.textMuted
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            implicitHeight: 34
                            color: parent.hovered ? colors.border : colors.cardStrong
                            border.color: colors.border
                            radius: 6
                        }
                    }

                    // Open in Folder
                    Button {
                        Layout.fillWidth: true
                        text: qsTr("Klasörde Göster")
                        onClicked: {
                            if (root.selectedIndex >= 0) {
                                libraryManager.openInFolder(root.selectedIndex)
                            }
                        }
                        contentItem: Text {
                            text: parent.text
                            color: colors.textMuted
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            implicitHeight: 34
                            color: parent.hovered ? colors.border : colors.cardStrong
                            border.color: colors.border
                            radius: 6
                        }
                    }

                    // Rename
                    Button {
                        Layout.fillWidth: true
                        text: qsTr("Yeniden Adlandır")
                        onClicked: {
                            if (root.selectedItem) {
                                root.newFileNameInput = root.selectedItem.fileName
                                root.isRenameOpen = true
                            }
                        }
                        contentItem: Text {
                            text: parent.text
                            color: colors.textMuted
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            implicitHeight: 34
                            color: parent.hovered ? colors.border : colors.cardStrong
                            border.color: colors.border
                            radius: 6
                        }
                    }

                    // Delete (Trash)
                    Button {
                        Layout.fillWidth: true
                        text: qsTr("Çöp Kutusuna Taşı")
                        onClicked: {
                            if (root.selectedIndex >= 0) {
                                root.trashSingleItem(root.selectedIndex)
                            }
                        }
                        contentItem: Text {
                            text: parent.text
                            color: colors.danger
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            implicitHeight: 34
                            color: parent.hovered ? "#450A0A" : colors.cardStrong
                            border.color: "#7F1D1D"
                            radius: 6
                        }
                    }

                    // Permanent Delete
                    Button {
                        Layout.fillWidth: true
                        text: qsTr("Kalıcı Olarak Sil...")
                        flat: true
                        onClicked: {
                            if (root.selectedIndex >= 0) {
                                root.requestPermanentDelete(root.selectedIndex)
                            }
                        }
                        contentItem: Text {
                            text: parent.text
                            color: colors.placeholder
                            font.pixelSize: 11
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }
        }
    }

    // Rename Dialog Overlay
    Rectangle {
        anchors.fill: parent
        color: "#000000"
        opacity: root.isRenameOpen ? 0.65 : 0.0
        visible: opacity > 0.0
        Behavior on opacity { NumberAnimation { duration: 150 } }

        MouseArea {
            anchors.fill: parent
            onClicked: root.isRenameOpen = false
        }
    }

    Rectangle {
        width: 360
        implicitHeight: renameCol.implicitHeight + 40
        anchors.centerIn: parent
        radius: 10
        color: colors.card
        border.color: colors.border
        border.width: 1
        visible: root.isRenameOpen
        z: 500

        ColumnLayout {
            id: renameCol
            anchors.fill: parent
            anchors.margins: 20
            spacing: 14

            Text {
                text: qsTr("Dosyayı Yeniden Adlandır")
                color: colors.text
                font.pixelSize: 15
                font.bold: true
            }

            TextField {
                id: renameField
                Layout.fillWidth: true
                text: root.newFileNameInput
                color: colors.text
                background: Rectangle {
                    implicitHeight: 36
                    color: colors.cardStrong
                    border.color: colors.border
                    radius: 6
                }
                onAccepted: {
                    var res = libraryManager.renameItem(root.selectedIndex, text)
                    if (res.success) {
                        root.isRenameOpen = false
                        snackbar.showMessage(qsTr("Dosya yeniden adlandırıldı."), "success")
                    } else {
                        snackbar.showMessage(res.error || qsTr("Yeniden adlandırılamadı."), "error")
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Item { Layout.fillWidth: true }

                Button {
                    text: qsTr("İptal")
                    onClicked: root.isRenameOpen = false
                    contentItem: Text { text: parent.text; color: colors.textSoft }
                    background: Rectangle { implicitHeight: 32; implicitWidth: 70; radius: 4; color: "transparent"; border.color: colors.border }
                }

                Button {
                    text: qsTr("Kaydet")
                    onClicked: {
                        var res = libraryManager.renameItem(root.selectedIndex, renameField.text)
                        if (res.success) {
                            root.isRenameOpen = false
                            snackbar.showMessage(qsTr("Dosya yeniden adlandırıldı."), "success")
                        } else {
                            snackbar.showMessage(res.error || qsTr("Yeniden adlandırılamadı."), "error")
                        }
                    }
                    contentItem: Text { text: parent.text; color: "#FFFFFF"; font.bold: true }
                    background: Rectangle { implicitHeight: 32; implicitWidth: 80; radius: 4; color: colors.accentA }
                }
            }
        }
    }

    // Annotation Editor Main View
    AnnotationEditor {
        id: annotationEditor
        colors: root.colors
        onSaved: function(path) {
            libraryManager.refresh()
            snackbar.showMessage(qsTr("Düzenlenen görsel kaydedildi."), "success")
        }
    }

    // Shared ConfirmDialog & AppSnackbar
    ConfirmDialog {
        id: confirmDialog
        colors: root.colors
    }

    AppSnackbar {
        id: snackbar
        colors: root.colors
    }
}
