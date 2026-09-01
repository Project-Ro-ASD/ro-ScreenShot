import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Effects

Item {
    id: root

    property var colors: ({})

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

            // Section 1: Kayıt Dizini (silgi iconuyla)
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: sec1Col.implicitHeight + 32
                color: colors.card
                border.color: colors.border
                border.width: 1
                radius: 10

                ColumnLayout {
                    id: sec1Col
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 14

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Image {
                            source: "assets/icon-folder.svg"
                            sourceSize.width: 18
                            sourceSize.height: 18
                            Layout.preferredWidth: 18
                            Layout.preferredHeight: 18
                            layer.enabled: true
                            layer.effect: MultiEffect {
                                colorization: 1.0
                                colorizationColor: colors.textSoft
                            }
                        }

                        Text {
                            text: qsTr("Kayıt Dizini ve Klasörleme")
                            color: colors.text
                            font.pixelSize: 15
                            font.bold: true
                            Layout.fillWidth: true
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        TextField {
                            id: saveDirField
                            Layout.fillWidth: true
                            text: settingsManager.saveDirectory
                            color: colors.text
                            font.pixelSize: 13
                            background: Rectangle {
                                implicitHeight: 38
                                color: colors.cardStrong
                                border.color: colors.border
                                radius: 6
                            }
                            onEditingFinished: {
                                settingsManager.saveDirectory = text
                            }
                        }

                        Button {
                            text: qsTr("Gözat...")
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
                                color: parent.hovered ? colors.accentA : colors.accentC
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
                            color: colors.textMuted
                            font.pixelSize: 13
                            leftPadding: parent.indicator.width + 8
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }

            // Section 2: İsimlendirme ve Format (etiket iconuyla)
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: sec2Col.implicitHeight + 32
                color: colors.card
                border.color: colors.border
                border.width: 1
                radius: 10

                ColumnLayout {
                    id: sec2Col
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 14

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Image {
                            source: "assets/icon-tags.svg"
                            sourceSize.width: 18
                            sourceSize.height: 18
                            Layout.preferredWidth: 18
                            Layout.preferredHeight: 18
                            layer.enabled: true
                            layer.effect: MultiEffect {
                                colorization: 1.0
                                colorizationColor: colors.textSoft
                            }
                        }

                        Text {
                            text: qsTr("Dosya İsimlendirme ve Görsel Formatı")
                            color: colors.text
                            font.pixelSize: 15
                            font.bold: true
                            Layout.fillWidth: true
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Text {
                            text: qsTr("Dosya Adı Şablonu (%Y: Yıl, %m: Ay, %d: Gün, %H: Saat, %M: Dakika, %S: Saniye)")
                            color: colors.textSoft
                            font.pixelSize: 12
                        }

                        TextField {
                            Layout.fillWidth: true
                            text: settingsManager.fileNameTemplate
                            color: colors.text
                            font.pixelSize: 13
                            background: Rectangle {
                                implicitHeight: 38
                                color: colors.cardStrong
                                border.color: colors.border
                                radius: 6
                            }
                            onTextChanged: settingsManager.fileNameTemplate = text
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 32
                            color: colors.shellAlt
                            radius: 4
                            border.color: colors.border

                            Text {
                                anchors.centerIn: parent
                                text: qsTr("Canlı Önizleme: %1").arg(settingsManager.previewFileName)
                                color: "#38BDF8"
                                font.pixelSize: 12
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 20

                        ColumnLayout {
                            spacing: 6
                            Text {
                                text: qsTr("Görsel Formatı")
                                color: colors.textSoft
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
                                            color: parent.checked ? "#FFFFFF" : colors.textSoft
                                            font.bold: parent.checked
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                        }
                                        background: Rectangle {
                                            implicitWidth: 70
                                            implicitHeight: 34
                                            color: parent.checked ? colors.accentB : (parent.hovered ? colors.border : colors.cardStrong)
                                            radius: 6
                                        }
                                    }
                                }
                            }
                        }

                        ColumnLayout {
                            visible: settingsManager.imageFormat === "jpg"
                            Layout.fillWidth: true
                            spacing: 4
                            Text {
                                text: qsTr("JPEG Kalitesi: %1%").arg(settingsManager.jpegQuality)
                                color: colors.textSoft
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

            // Section 3: Çekim Sonrası İş Akışı (hızlı ayarlar iconuyla)
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: sec3Col.implicitHeight + 32
                color: colors.card
                border.color: colors.border
                border.width: 1
                radius: 10

                ColumnLayout {
                    id: sec3Col
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Image {
                            source: "assets/icon-bolt.svg"
                            sourceSize.width: 18
                            sourceSize.height: 18
                            Layout.preferredWidth: 18
                            Layout.preferredHeight: 18
                            layer.enabled: true
                            layer.effect: MultiEffect {
                                colorization: 1.0
                                colorizationColor: colors.textSoft
                            }
                        }

                        Text {
                            text: qsTr("Çekim Sonrası Otomatik İşlemler")
                            color: colors.text
                            font.pixelSize: 15
                            font.bold: true
                            Layout.fillWidth: true
                        }
                    }

                    CheckBox {
                        text: qsTr("Otomatik olarak panoya kopyala (Clipboard)")
                        checked: settingsManager.autoCopyToClipboard
                        onToggled: settingsManager.autoCopyToClipboard = checked
                        contentItem: Text {
                            text: parent.text
                            color: colors.textMuted
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
                            color: colors.textMuted
                            font.pixelSize: 13
                            leftPadding: parent.indicator.width + 8
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    CheckBox {
                        text: qsTr("Çekim sonrası sağ altta yüzen Önizleme kartı göster (Floating Thumbnail)")
                        checked: settingsManager.showFloatingThumbnail
                        onToggled: settingsManager.showFloatingThumbnail = checked
                        contentItem: Text {
                            text: parent.text
                            color: colors.textMuted
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
                            color: colors.textMuted
                            font.pixelSize: 13
                            leftPadding: parent.indicator.width + 8
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }

            // Section 4: Sniper ve Büyüteç (nişangah iconuyla)
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: sec4Col.implicitHeight + 32
                color: colors.card
                border.color: colors.border
                border.width: 1
                radius: 10

                ColumnLayout {
                    id: sec4Col
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Image {
                            source: "assets/icon-crosshair.svg"
                            sourceSize.width: 18
                            sourceSize.height: 18
                            Layout.preferredWidth: 18
                            Layout.preferredHeight: 18
                            layer.enabled: true
                            layer.effect: MultiEffect {
                                colorization: 1.0
                                colorizationColor: colors.textSoft
                            }
                        }

                        Text {
                            text: qsTr("Sniper Seçim Katmanı ve Büyüteç (Loupe)")
                            color: colors.text
                            font.pixelSize: 15
                            font.bold: true
                            Layout.fillWidth: true
                        }
                    }

                    CheckBox {
                        text: qsTr("Piksel Büyüteci ve Canlı Renk Damlalığını (Loupe) aktif et")
                        checked: settingsManager.magnifierEnabled
                        onToggled: settingsManager.magnifierEnabled = checked
                        contentItem: Text {
                            text: parent.text
                            color: colors.textMuted
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
                            text: qsTr("Büyüteç Yakınlaştirması: %1x").arg(settingsManager.magnifierZoom)
                            color: colors.textSoft
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
                            color: colors.textMuted
                            font.pixelSize: 13
                            leftPadding: parent.indicator.width + 8
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }

            // Section 5: Dil Seçenekleri (dil iconuyla + buton grubu)
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: sec5Col.implicitHeight + 32
                color: colors.card
                border.color: colors.border
                border.width: 1
                radius: 10

                ColumnLayout {
                    id: sec5Col
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 14

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Image {
                            source: "assets/icon-language.svg"
                            sourceSize.width: 18
                            sourceSize.height: 18
                            Layout.preferredWidth: 18
                            Layout.preferredHeight: 18
                            layer.enabled: true
                            layer.effect: MultiEffect {
                                colorization: 1.0
                                colorizationColor: colors.textSoft
                            }
                        }

                        Text {
                            text: qsTr("Dil ve Arayüz Seçenekleri")
                            color: colors.text
                            font.pixelSize: 15
                            font.bold: true
                            Layout.fillWidth: true
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 16

                        Text {
                            text: qsTr("Uygulama Dili")
                            color: colors.textSoft
                            font.pixelSize: 13
                        }

                        RowLayout {
                            spacing: 8
                            Repeater {
                                model: LanguageManager.availableLanguages
                                delegate: Button {
                                    text: modelData.nativeLabel
                                    checkable: true
                                    checked: LanguageManager.currentLanguage === modelData.code
                                    onClicked: LanguageManager.setCurrentLanguage(modelData.code)
                                    contentItem: Text {
                                        text: parent.text
                                        color: parent.checked ? "#FFFFFF" : colors.textSoft
                                        font.bold: parent.checked
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    background: Rectangle {
                                        implicitWidth: 80
                                        implicitHeight: 34
                                        color: parent.checked ? colors.accentB : (parent.hovered ? colors.border : colors.cardStrong)
                                        radius: 6
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Section 6: Uygulama Teması (ro-Control style theme toggle)
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: sec6Col.implicitHeight + 32
                color: colors.card
                border.color: colors.border
                border.width: 1
                radius: 10

                ColumnLayout {
                    id: sec6Col
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 14

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Image {
                            source: "assets/icon-theme.svg"
                            sourceSize.width: 18
                            sourceSize.height: 18
                            Layout.preferredWidth: 18
                            Layout.preferredHeight: 18
                            layer.enabled: true
                            layer.effect: MultiEffect {
                                colorization: 1.0
                                colorizationColor: colors.textSoft
                            }
                        }

                        Text {
                            text: qsTr("Tema")
                            color: colors.text
                            font.pixelSize: 15
                            font.bold: true
                            Layout.fillWidth: true
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 16

                        Text {
                            text: qsTr("Uygulama Teması")
                            color: colors.textSoft
                            font.pixelSize: 13
                        }

                        RowLayout {
                            spacing: 8
                            Repeater {
                                model: UiPreferencesManager.availableThemeModes
                                delegate: Button {
                                    required property var modelData
                                    implicitWidth: 100
                                    implicitHeight: 34
                                    text: modelData.label
                                    checkable: true
                                    checked: UiPreferencesManager.themeMode === modelData.code
                                    onClicked: UiPreferencesManager.setThemeMode(modelData.code)
                                    contentItem: Text {
                                        text: parent.text
                                        color: parent.checked ? "#FFFFFF" : colors.text
                                        font.bold: parent.checked
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    background: Rectangle {
                                        color: parent.checked ? colors.accentA : (parent.hovered ? colors.border : colors.cardStrong)
                                        border.color: parent.checked ? colors.accentA : colors.border
                                        border.width: 1
                                        radius: 6
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Reset Button
            Button {
                text: qsTr("Varsayılan Ayarlara Sıfırla")
                Layout.alignment: Qt.AlignRight
                onClicked: settingsManager.resetToDefaults()
                contentItem: Text {
                    text: parent.text
                    color: colors.danger
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    implicitWidth: 200
                    implicitHeight: 38
                    color: parent.hovered ? "#450A0A" : colors.cardStrong
                    border.color: "#7F1D1D"
                    radius: 6
                }
            }

            Item { height: 20 }
        }
    }
}
