import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Effects
import "components"

Item {
    id: root

    property var colors: ({})
    property bool isSaveDirValid: settingsManager.saveDirectory.length > 0

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
            snackbar.showMessage(qsTr("Kayıt klasörü güncellendi."), "success")
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
            spacing: 20

            Item { height: 10 }

            // Section 1: Kayıt Dizini
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
                            source: "qrc:/qt/qml/ro_screenshot/assets/icon-folder.svg"
                            sourceSize.width: 18
                            sourceSize.height: 18
                            Layout.preferredWidth: 18
                            Layout.preferredHeight: 18
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
                                snackbar.showMessage(qsTr("Kayıt dizini kaydedildi."), "success")
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

                    InlineStatus {
                        status: root.isSaveDirValid ? "success" : "error"
                        text: root.isSaveDirValid ? qsTr("Kayıt dizini geçerli ve yazılabilir.") : qsTr("Geçersiz veya erişilemeyen kayıt dizini.")
                        colors: root.colors
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

            // Section 2: İsimlendirme ve Format
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
                            source: "qrc:/qt/qml/ro_screenshot/assets/icon-tags.svg"
                            sourceSize.width: 18
                            sourceSize.height: 18
                            Layout.preferredWidth: 18
                            Layout.preferredHeight: 18
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
                            id: templateField
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
                            onEditingFinished: {
                                settingsManager.fileNameTemplate = text
                            }
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

                    // Format Trade-offs Explanation Card
                    Rectangle {
                        Layout.fillWidth: true
                        implicitHeight: formatDescText.implicitHeight + 16
                        color: colors.shellAlt
                        radius: 6
                        border.color: colors.border

                        Text {
                            id: formatDescText
                            anchors.fill: parent
                            anchors.margins: 10
                            text: {
                                if (settingsManager.imageFormat === "png")
                                    return qsTr("ℹ PNG: Kayıpsız sıkıştırma ve şeffaflık desteği sunar. En yüksek kalite için önerilir.")
                                if (settingsManager.imageFormat === "jpg")
                                    return qsTr("ℹ JPEG: Yüksek sıkıştırma oranı ve ayarlanabilir kalite ile küçük dosya boyutu sağlar.")
                                return qsTr("ℹ WebP: Modern web standardı, PNG ve JPEG'e göre %30 daha küçük dosya boyutu sağlar.")
                            }
                            color: colors.textMuted
                            font.pixelSize: 11
                            wrapMode: Text.WordWrap
                        }
                    }
                }
            }

            // Section 3: Çekim Sonrası İş Akışı
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
                            source: "qrc:/qt/qml/ro_screenshot/assets/icon-bolt.svg"
                            sourceSize.width: 18
                            sourceSize.height: 18
                            Layout.preferredWidth: 18
                            Layout.preferredHeight: 18
                        }

                        Text {
                            text: qsTr("Çekim Sonrası Otomatik İşlemler")
                            color: colors.text
                            font.pixelSize: 15
                            font.bold: true
                            Layout.fillWidth: true
                        }
                    }

                    // Warning if both auto-save and auto-copy are disabled
                    InlineStatus {
                        visible: !settingsManager.autoCopyToClipboard && !settingsManager.autoSaveToDisk
                        status: "warning"
                        text: qsTr("Uyarı: Hem panoya kopyalama hem diske kaydetme kapalı. Çekilen görüntüler saklanmayacaktır.")
                        colors: root.colors
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

            // Section 4: Sniper ve Büyüteç
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
                            source: "qrc:/qt/qml/ro_screenshot/assets/icon-crosshair.svg"
                            sourceSize.width: 18
                            sourceSize.height: 18
                            Layout.preferredWidth: 18
                            Layout.preferredHeight: 18
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
                            text: qsTr("Büyüteç Yakınlaştırması: %1x").arg(settingsManager.magnifierZoom)
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

            // Section 5: Dil ve Arayüz Seçenekleri
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
                            source: "qrc:/qt/qml/ro_screenshot/assets/icon-language.svg"
                            sourceSize.width: 18
                            sourceSize.height: 18
                            Layout.preferredWidth: 18
                            Layout.preferredHeight: 18
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

            // Section 6: Tema Seçenekleri
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
                            source: "qrc:/qt/qml/ro_screenshot/assets/icon-theme.svg"
                            sourceSize.width: 18
                            sourceSize.height: 18
                            Layout.preferredWidth: 18
                            Layout.preferredHeight: 18
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

            // Section 7: Önizleme Önbelleği Temizleme
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 64
                color: colors.card
                border.color: colors.border
                border.width: 1
                radius: 10

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12

                    Text {
                        text: qsTr("Küçük Resim (Thumbnail) Önbelleğini Temizle")
                        color: colors.text
                        font.pixelSize: 13
                        Layout.fillWidth: true
                    }

                    Button {
                        text: qsTr("Önbelleği Boşalt")
                        onClicked: {
                            if (libraryManager.clearThumbnailCache()) {
                                snackbar.showMessage(qsTr("Küçük resim önbelleği temizlendi."), "success")
                            }
                        }
                        contentItem: Text {
                            text: parent.text
                            color: colors.textMuted
                            font.pixelSize: 12
                        }
                        background: Rectangle {
                            implicitWidth: 130
                            implicitHeight: 34
                            color: parent.hovered ? colors.border : colors.cardStrong
                            border.color: colors.border
                            radius: 6
                        }
                    }
                }
            }

            // Reset Button with Confirmation
            Button {
                text: qsTr("Varsayılan Ayarlara Sıfırla")
                Layout.alignment: Qt.AlignRight
                onClicked: {
                    resetConfirmDialog.open(
                        qsTr("Ayarları Sıfırla"),
                        qsTr("Tüm ayarları fabrika varsayılanlarına döndürmek istediğinizden emin misiniz?"),
                        true,
                        function() {
                            settingsManager.resetToDefaults()
                            snackbar.showMessage(qsTr("Ayarlar varsayılana sıfırlandı."), "info")
                        }
                    )
                }
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

    ConfirmDialog {
        id: resetConfirmDialog
        colors: root.colors
    }

    AppSnackbar {
        id: snackbar
        colors: root.colors
    }
}
