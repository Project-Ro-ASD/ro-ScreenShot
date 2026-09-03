import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

Item {
    id: root

    property var colors: ({})
    property int selectedIndex: -1
    property var selectedItem: selectedIndex >= 0 ? libraryManager.getItem(selectedIndex) : null

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Left / Main Content: Search, Filter, Grid
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: parent.height
            spacing: 16

            // Filter & Search Toolbar
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 64
                color: colors.shell
                border.color: colors.border
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 20
                    anchors.rightMargin: 20
                    spacing: 14

                    // Search input
                    Rectangle {
                        Layout.preferredWidth: 260
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
                                source: "assets/icon-search.svg"
                                sourceSize.width: 13
                                sourceSize.height: 13
                                Layout.preferredWidth: 13
                                Layout.preferredHeight: 13
                                layer.enabled: true
                                layer.effect: MultiEffect {
                                    colorization: 1.0
                                    colorizationColor: colors.textSoft
                                }
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
                                { label: qsTr("Bu Ay"), filter: 4 }
                            ]
                            delegate: Button {
                                text: modelData.label
                                checkable: true
                                checked: libraryManager.dateFilter === modelData.filter
                                onClicked: libraryManager.dateFilter = modelData.filter
                                contentItem: Text {
                                    text: parent.text
                                    color: parent.checked ? "#FFFFFF" : colors.textSoft
                                    font.pixelSize: 12
                                    font.bold: parent.checked
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                background: Rectangle {
                                    implicitWidth: 70
                                    implicitHeight: 32
                                    color: parent.checked ? colors.accentB : (parent.hovered ? colors.border : colors.cardStrong)
                                    radius: 6
                                }
                            }
                        }
                    }

                    Item { Layout.fillWidth: true }

                    // Storage Badge (icon + text)
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
                                source: "assets/icon-folder.svg"
                                sourceSize.width: 13
                                sourceSize.height: 13
                                Layout.preferredWidth: 13
                                Layout.preferredHeight: 13
                                layer.enabled: true
                                layer.effect: MultiEffect {
                                    colorization: 1.0
                                    colorizationColor: colors.textSoft
                                }
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

                    // Refresh Button (icon-only)
                    Button {
                        flat: true
                        onClicked: libraryManager.refresh()
                        contentItem: Image {
                            source: "assets/icon-refresh.svg"
                            sourceSize.width: 16
                            sourceSize.height: 16
                            layer.enabled: true
                            layer.effect: MultiEffect {
                                colorization: 1.0
                                colorizationColor: colors.textMuted
                            }
                        }
                        background: Rectangle {
                            implicitWidth: 32
                            implicitHeight: 32
                            color: parent.hovered ? colors.border : "transparent"
                            radius: 6
                        }

                        ToolTip {
                            text: qsTr("Yenile")
                            delay: 300
                            timeout: 5000
                        }
                    }
                }
            }

            // Grid or Empty State
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                // Empty State (icon + text, no emoji)
                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 12
                    visible: libraryManager.count === 0

                    Image {
                        source: "assets/icon-gallery-empty.svg"
                        sourceSize.width: 48
                        sourceSize.height: 48
                        Layout.alignment: Qt.AlignHCenter
                        layer.enabled: true
                        layer.effect: MultiEffect {
                            colorization: 1.0
                            colorizationColor: colors.muted
                        }
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
                    anchors.margins: 20
                    cellWidth: 230
                    cellHeight: 200
                    clip: true
                    visible: libraryManager.count > 0
                    model: libraryManager

                    delegate: Rectangle {
                        width: 216
                        height: 186
                        color: root.selectedIndex === index ? colors.selected : (itemMouse.containsMouse ? colors.cardStrong : colors.card)
                        border.color: root.selectedIndex === index ? colors.accentB : (itemMouse.containsMouse ? colors.muted : colors.border)
                        border.width: root.selectedIndex === index ? 2 : 1
                        radius: 8

                        MouseArea {
                            id: itemMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                root.selectedIndex = index
                            }
                            onDoubleClicked: {
                                libraryManager.openFile(index)
                            }
                        }

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 6

                            // Thumbnail Image
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
            Layout.preferredWidth: root.selectedItem ? 300 : 0
            Layout.fillHeight: true
            color: colors.shell
            border.color: colors.border
            border.width: 1
            visible: root.selectedItem !== null
            clip: true

            Behavior on Layout.preferredWidth {
                NumberAnimation { duration: 150 }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 16
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
                    Layout.preferredHeight: 170
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

                // Actions (icon+label buttons)
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    // Copy to Clipboard
                    Button {
                        Layout.fillWidth: true
                        text: qsTr("Panoya Kopyala")
                        onClicked: {
                            if (root.selectedIndex >= 0) {
                                libraryManager.copyToClipboard(root.selectedIndex)
                            }
                        }
                        contentItem: RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            spacing: 10
                            Image {
                                source: "assets/icon-copy.svg"
                                sourceSize.width: 18
                                sourceSize.height: 18
                                Layout.preferredWidth: 18
                                Layout.preferredHeight: 18
                                layer.enabled: true
                                layer.effect: MultiEffect {
                                    colorization: 1.0
                                    colorizationColor: "#FFFFFF"
                                }
                            }
                            Text {
                                text: parent.parent ? parent.parent.text : ""
                                color: "#FFFFFF"
                                font.bold: true
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                                Layout.fillWidth: true
                            }
                        }
                        background: Rectangle {
                            implicitHeight: 36
                            color: parent.hovered ? colors.accentC : colors.accentA
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
                        contentItem: RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            spacing: 10
                            Image {
                                source: "assets/icon-folder.svg"
                                sourceSize.width: 18
                                sourceSize.height: 18
                                Layout.preferredWidth: 18
                                Layout.preferredHeight: 18
                                layer.enabled: true
                                layer.effect: MultiEffect {
                                    colorization: 1.0
                                    colorizationColor: colors.textMuted
                                }
                            }
                            Text {
                                text: parent.parent ? parent.parent.text : ""
                                color: colors.textMuted
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                                Layout.fillWidth: true
                            }
                        }
                        background: Rectangle {
                            implicitHeight: 36
                            color: parent.hovered ? colors.border : colors.cardStrong
                            border.color: colors.border
                            radius: 6
                        }
                    }

                    // Delete
                    Button {
                        Layout.fillWidth: true
                        text: qsTr("Görseli Sil")
                        onClicked: {
                            if (root.selectedIndex >= 0) {
                                libraryManager.deleteItem(root.selectedIndex)
                                root.selectedIndex = -1
                            }
                        }
                        contentItem: RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            spacing: 10
                            Image {
                                source: "assets/icon-trash.svg"
                                sourceSize.width: 18
                                sourceSize.height: 18
                                Layout.preferredWidth: 18
                                Layout.preferredHeight: 18
                                layer.enabled: true
                                layer.effect: MultiEffect {
                                    colorization: 1.0
                                    colorizationColor: colors.danger
                                }
                            }
                            Text {
                                text: parent.parent ? parent.parent.text : ""
                                color: colors.danger
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                                Layout.fillWidth: true
                            }
                        }
                        background: Rectangle {
                            implicitHeight: 36
                            color: parent.hovered ? "#450A0A" : colors.cardStrong
                            border.color: "#7F1D1D"
                            radius: 6
                        }
                    }
                }
            }
        }
    }
}
