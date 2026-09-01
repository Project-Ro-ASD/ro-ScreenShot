import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Item {
    id: root

    FolderDialog {
        id: folderDialog
        title: qsTr("Ekran Görüntüleri Kayıt Klasörünü Seçin")
        currentFolder: "file://" + settingsManager.saveDirectory
        onAccepted: {
            var path = selectedFolder.toString()
            if (path.startsWith("file://")) {
                path = path.substring(7)
            }
            settingsManager.saveDirectory = path
        }
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: Math.min(parent.width - 40, 780)
        contentHeight: settingsColumn.implicitHeight + 40
        clip: true

        ColumnLayout {
            id: settingsColumn
            width: Math.min(parent.width - 40, 780)
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 24

            Item { height: 10 }

            // Section 1: Kayıt ve Dizin Ayarları
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: sec1Col.implicitHeight + 32
                color: "#131D31"
                border.color: "#334155"
                border.width: 1
                radius: 10

                ColumnLayout {
                    id: sec1Col
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 14

                    Text {
                        text: qsTr("📁 Kayıt Dizini ve Klasörleme")
                        color: "#F8FAFC"
                        font.pixelSize: 15
                        font.bold: true
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        TextField {
                            id: saveDirField
                            Layout.fillWidth: true
                            text: settingsManager.saveDirectory
                            color: "#F8FAFC"
                            font.pixelSize: 13
                            background: Rectangle {
                                implicitHeight: 38
                                color: "#1E293B"
                                border.color: "#334155"
                                radius: 6
                            }
                            onEditingFinished: {
                                settingsManager.saveDirectory = text
                            }
                        }

                        Button {
                            text: qsTr("📂 Gözat...")
                            onClicked: folderDialog.open()
                            contentItem: Text {
                                text: parent.text
                                color: "#FFFFFF"
                                font.bold: true
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            background: Rectangle {
                                implicitWidth: 100
                                implicitHeight: 38
                                color: parent.hovered ? "#2563EB" : "#1D4ED8"
                                radius: 6
                            }
                        }
                    }

                    CheckBox {
                        text: qsTr("Görselleri Yıl-Ay bazlı alt klasörlerde grupla (Örn: 2026-09/)")
                        checked: settingsManager.createSubfoldersByMonth
                        onToggled: settingsManager.createSubfoldersByMonth = checked
                        contentItem: Text {
                            text: parent.text
                            color: "#CBD5E1"
                            font.pixelSize: 13
                            leftPadding: parent.indicator.width + 8
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }

            // Section 2: İsimlendirme ve Format
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: sec2Col.implicitHeight + 32
                color: "#131D31"
                border.color: "#334155"
                border.width: 1
                radius: 10

                ColumnLayout {
                    id: sec2Col
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 14

                    Text {
                        text: qsTr("🏷️ Dosya İsimlendirme ve Görsel Formatı")
                        color: "#F8FAFC"
                        font.pixelSize: 15
                        font.bold: true
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Text {
                            text: qsTr("Dosya Adı Şablonu (%Y: Yıl, %m: Ay, %d: Gün, %H: Saat, %M: Dakika, %S: Saniye)")
                            color: "#94A3B8"
                            font.pixelSize: 12
                        }

                        TextField {
                            Layout.fillWidth: true
                            text: settingsManager.fileNameTemplate
                            color: "#F8FAFC"
                            font.pixelSize: 13
                            background: Rectangle {
                                implicitHeight: 38
                                color: "#1E293B"
                                border.color: "#334155"
                                radius: 6
                            }
                            onTextChanged: settingsManager.fileNameTemplate = text
                        }

                        // Live Preview Box
                        Rectangle {
                            Layout.fillWidth: true
                            height: 32
                            color: "#0F172A"
                            radius: 4
                            border.color: "#1E293B"
                            Text {
                                anchors.centerIn: parent
                                text: qsTr("Canlı Önizleme: %1").arg(settingsManager.previewFileName)
                                color: "#38BDF8"
                                font.pixelSize: 12
                            }
                        }
                    }

                    // Format Selection
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 20

                        ColumnLayout {
                            spacing: 6
                            Text {
                                text: qsTr("Görsel Formatı")
                                color: "#94A3B8"
                                font.pixelSize: 12
                            }
                            RowLayout {
                                spacing: 8
                                Repeater {
                                    model: ["png", "jpg", "webp"]
                                    delegate: Button {
                                        text: modelData.toUpperCase()
                                        checkable: true
                                        checked: settingsManager.imageFormat === modelData
                                        onClicked: settingsManager.imageFormat = modelData
                                        contentItem: Text {
                                            text: parent.text
                                            color: parent.checked ? "#FFFFFF" : "#94A3B8"
                                            font.bold: parent.checked
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                        }
                                        background: Rectangle {
                                            implicitWidth: 70
                                            implicitHeight: 34
                                            color: parent.checked ? "#3B82F6" : (parent.hovered ? "#334155" : "#1E293B")
                                            radius: 6
                                        }
                                    }
                                }
                            }
                        }

                        // JPEG Quality Slider
                        ColumnLayout {
                            visible: settingsManager.imageFormat === "jpg"
                            Layout.fillWidth: true
                            spacing: 4
                            Text {
                                text: qsTr("JPEG Kalitesi: %%%1").arg(settingsManager.jpegQuality)
                                color: "#94A3B8"
                                font.pixelSize: 12
                            }
                            Slider {
                                Layout.fillWidth: true
                                from: 10
                                to: 100
                                stepSize: 5
                                value: settingsManager.jpegQuality
                                onMoved: settingsManager.jpegQuality = value
                            }
                        }
                    }
                }
            }

            // Section 3: Çekim Sonrası İş Akışı
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: sec3Col.implicitHeight + 32
                color: "#131D31"
                border.color: "#334155"
                border.width: 1
                radius: 10

                ColumnLayout {
                    id: sec3Col
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12

                    Text {
                        text: qsTr("⚡ Çekim Sonrası Otomatik İşlemler")
                        color: "#F8FAFC"
                        font.pixelSize: 15
                        font.bold: true
                    }

                    CheckBox {
                        text: qsTr("Otomatik olarak panoya kopyala (Clipboard)")
                        checked: settingsManager.autoCopyToClipboard
                        onToggled: settingsManager.autoCopyToClipboard = checked
                        contentItem: Text {
                            text: parent.text
                            color: "#CBD5E1"
                            font.pixelSize: 13
                            leftPadding: parent.indicator.width + 8
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    CheckBox {
                        text: qsTr("Otomatik olarak diske kaydet")
                        checked: settingsManager.autoSaveToDisk
                        onToggled: settingsManager.autoSaveToDisk = checked
                        contentItem: Text {
                            text: parent.text
                            color: "#CBD5E1"
                            font.pixelSize: 13
                            leftPadding: parent.indicator.width + 8
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    CheckBox {
                        text: qsTr("Çekim sonrası sağ altta yüzen önizleme kartı göster (Floating Thumbnail)")
                        checked: settingsManager.showFloatingThumbnail
                        onToggled: settingsManager.showFloatingThumbnail = checked
                        contentItem: Text {
                            text: parent.text
                            color: "#CBD5E1"
                            font.pixelSize: 13
                            leftPadding: parent.indicator.width + 8
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    CheckBox {
                        text: qsTr("Masaüstü bildirimi göster")
                        checked: settingsManager.showNotification
                        onToggled: settingsManager.showNotification = checked
                        contentItem: Text {
                            text: parent.text
                            color: "#CBD5E1"
                            font.pixelSize: 13
                            leftPadding: parent.indicator.width + 8
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }

            // Section 4: Sniper ve Büyüteç Tercihleri
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: sec4Col.implicitHeight + 32
                color: "#131D31"
                border.color: "#334155"
                border.width: 1
                radius: 10

                ColumnLayout {
                    id: sec4Col
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12

                    Text {
                        text: qsTr("🎯 Sniper Seçim Katmanı ve Büyüteç (Loupe)")
                        color: "#F8FAFC"
                        font.pixelSize: 15
                        font.bold: true
                    }

                    CheckBox {
                        text: qsTr("Piksel Büyüteci ve Canlı Renk Damlalığını (Loupe) aktif et")
                        checked: settingsManager.magnifierEnabled
                        onToggled: settingsManager.magnifierEnabled = checked
                        contentItem: Text {
                            text: parent.text
                            color: "#CBD5E1"
                            font.pixelSize: 13
                            leftPadding: parent.indicator.width + 8
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    RowLayout {
                        visible: settingsManager.magnifierEnabled
                        Layout.fillWidth: true
                        spacing: 16

                        Text {
                            text: qsTr("Büyüteç Yakınlaştırması: %1x").arg(settingsManager.magnifierZoom)
                            color: "#94A3B8"
                            font.pixelSize: 12
                        }

                        Slider {
                            Layout.fillWidth: true
                            from: 2
                            to: 16
                            stepSize: 2
                            value: settingsManager.magnifierZoom
                            onMoved: settingsManager.magnifierZoom = value
                        }
                    }

                    CheckBox {
                        text: qsTr("Seçim tamamlandığında Sniper katmanını otomatik kapat")
                        checked: settingsManager.closeOverlayOnCapture
                        onToggled: settingsManager.closeOverlayOnCapture = checked
                        contentItem: Text {
                            text: parent.text
                            color: "#CBD5E1"
                            font.pixelSize: 13
                            leftPadding: parent.indicator.width + 8
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }

            // Reset Button
            Button {
                text: qsTr("🔄 Varsayılan Ayarlara Sıfırla")
                Layout.alignment: Qt.AlignRight
                onClicked: settingsManager.resetToDefaults()
                contentItem: Text {
                    text: parent.text
                    color: "#EF4444"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    implicitWidth: 200
                    implicitHeight: 38
                    color: parent.hovered ? "#450A0A" : "#1E293B"
                    border.color: "#7F1D1D"
                    radius: 6
                }
            }

            Item { height: 20 }
        }
    }
}
