import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

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
                color: "#131D31"
                border.color: "#334155"
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
                        color: "#1E293B"
                        border.color: searchField.activeFocus ? "#3B82F6" : "#334155"
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
                                color: "#94A3B8"
                            }

                            TextField {
                                id: searchField
                                Layout.fillWidth: true
                                placeholderText: qsTr("Görsellerde ara...")
                                placeholderTextColor: "#64748B"
                                color: "#F8FAFC"
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
                                    color: "#94A3B8"
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
                                    color: parent.checked ? "#FFFFFF" : "#94A3B8"
                                    font.pixelSize: 12
                                    font.bold: parent.checked
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                background: Rectangle {
                                    implicitWidth: 70
                                    implicitHeight: 32
                                    color: parent.checked ? "#3B82F6" : (parent.hovered ? "#334155" : "#1E293B")
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
                        color: "#1E293B"
                        border.color: "#334155"
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
                                color: "#94A3B8"
                            }

                            Text {
                                id: storageText
                                anchors.centerIn: parent
                                text: qsTr("%1 Dahilinde").arg(libraryManager.totalStorageSize)
                                color: "#94A3B8"
                                font.pixelSize: 11
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
                            color: "#E2E8F0"
                        }
                        background: Rectangle {
                            implicitWidth: 32
                            implicitHeight: 32
                            color: parent.hovered ? "#334155" : "transparent"
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
                        color: "#475569"
                    }

                    Text {
                        text: qsTr("Henüz ekran görüntüsü bulunamadı.")
                        color: "#F8FAFC"
                        font.pixelSize: 16
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Text {
                        text: qsTr("Kayıt klasörü: %1").arg(settingsManager.saveDirectory)
                        color: "#64748B"
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
                        color: root.selectedIndex === index ? "#1E3A8A" : (itemMouse.containsMouse ? "#1E293B" : "#131D31")
                        border.color: root.selectedIndex === index ? "#3B82F6" : (itemMouse.containsMouse ? "#475569" : "#334155")
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
                                color: "#0F172A"
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
                                    color: "#F8FAFC"
                                    font.pixelSize: 11
                                    font.bold: true
                                    elide: Text.ElideMiddle
                                    Layout.fillWidth: true
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    Text {
                                        text: model.formattedDate
                                        color: "#64748B"
                                        font.pixelSize: 10
                                    }
                                    Item { Layout.fillWidth: true }
                                    Text {
                                        text: model.formattedSize
                                        color: "#94A3B8"
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
            color: "#131D31"
            border.color: "#334155"
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
                        color: "#F8FAFC"
                        font.pixelSize: 15
                        font.bold: true
                    }
                    Item { Layout.fillWidth: true }
                    Button {
                        flat: true
                        onClicked: root.selectedIndex = -1
                        contentItem: Text {
                            text: "✕"
                            color: "#94A3B8"
                            font.pixelSize: 14
                        }
                        background: Item {}
                    }
                }

                // Large Preview Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 170
                    color: "#0F172A"
                    radius: 8
                    border.color: "#334155"
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
                            color: "#64748B"
                            font.pixelSize: 11
                        }
                        Text {
                            text: root.selectedItem ? root.selectedItem.fileName : ""
                            color: "#F8FAFC"
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
                                color: "#64748B"
                                font.pixelSize: 11
                            }
                            Text {
                                text: root.selectedItem ? root.selectedItem.resolution : ""
                                color: "#E2E8F0"
                                font.pixelSize: 12
                            }
                        }
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Text {
                                text: qsTr("Boyut")
                                color: "#64748B"
                                font.pixelSize: 11
                            }
                            Text {
                                text: root.selectedItem ? root.selectedItem.formattedSize : ""
                                color: "#E2E8F0"
                                font.pixelSize: 12
                            }
                        }
                    }

                    ColumnLayout {
                        spacing: 2
                        Text {
                            text: qsTr("Oluşturulma Tarihi")
                            color: "#64748B"
                            font.pixelSize: 11
                        }
                        Text {
                            text: root.selectedItem ? root.selectedItem.formattedDate : ""
                            color: "#E2E8F0"
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
                            }
                            Text {
                                text: parent.Parent ? parent.text : ""
                                color: "#FFFFFF"
                                font.bold: true
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                                Layout.fillWidth: true
                            }
                        }
                        background: Rectangle {
                            implicitHeight: 36
                            color: parent.hovered ? "#1D4ED8" : "#2563EB"
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
                            }
                            Text {
                                text: parent.Parent ? parent.text : ""
                                color: "#E2E8F0"
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                                Layout.fillWidth: true
                            }
                        }
                        background: Rectangle {
                            implicitHeight: 36
                            color: parent.hovered ? "#334155" : "#1E293B"
                            border.color: "#334155"
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
                            }
                            Text {
                                text: parent.Parent ? parent.text : ""
                                color: "#EF4444"
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                                Layout.fillWidth: true
                            }
                        }
                        background: Rectangle {
                            implicitHeight: 36
                            color: parent.hovered ? "#450A0A" : "#1E293B"
                            border.color: "#7F1D1D"
                            radius: 6
                        }
                    }
                }
            }
        }
    }
}
