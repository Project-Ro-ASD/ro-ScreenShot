#include "ShortcutManager.hpp"

namespace ro_screenshot {

namespace {
constexpr const char *SHORTCUT_GROUP = "Shortcuts";
}

ShortcutManager::ShortcutManager(QObject *parent)
    : QObject(parent), m_settings("ro-asd", "ro-screenshot") {
  initDefaultDefinitions();
  load();
}

void ShortcutManager::initDefaultDefinitions() {
  m_shortcuts.insert(
      ShortcutAction::CaptureRegion,
      {ShortcutAction::CaptureRegion, QStringLiteral("region"),
       tr("Bölge Yakalama"), tr("Seçilen dikdörtgen alanı yakalar."),
       QStringLiteral("Ctrl+Shift+Print"), QStringLiteral("Ctrl+Shift+Print")});

  m_shortcuts.insert(ShortcutAction::CaptureFullscreen,
                     {ShortcutAction::CaptureFullscreen,
                      QStringLiteral("fullscreen"), tr("Tam Ekran Yakalama"),
                      tr("Tüm masaüstü ekranını yakalar."),
                      QStringLiteral("Print"), QStringLiteral("Print")});

  m_shortcuts.insert(ShortcutAction::CaptureWindow,
                     {ShortcutAction::CaptureWindow, QStringLiteral("window"),
                      tr("Pencere Yakalama"), tr("Aktif pencereyi yakalar."),
                      QStringLiteral("Alt+Print"),
                      QStringLiteral("Alt+Print")});

  m_shortcuts.insert(ShortcutAction::CaptureLastRegion,
                     {ShortcutAction::CaptureLastRegion,
                      QStringLiteral("last_region"), tr("Son Bölgeyi Yakala"),
                      tr("En son seçilen bölgeyi anında tekrar yakalar."),
                      QStringLiteral("Shift+Print"),
                      QStringLiteral("Shift+Print")});

  m_shortcuts.insert(
      ShortcutAction::CaptureRegionCopyOnly,
      {ShortcutAction::CaptureRegionCopyOnly,
       QStringLiteral("region_copy_only"), tr("Bölge (Yalnızca Pano)"),
       tr("Bölgeyi yakalar ve sadece panoya kopyalar."),
       QStringLiteral("Ctrl+Print"), QStringLiteral("Ctrl+Print")});

  m_shortcuts.insert(
      ShortcutAction::CaptureRegionSaveOnly,
      {ShortcutAction::CaptureRegionSaveOnly,
       QStringLiteral("region_save_only"), tr("Bölge (Yalnızca Disk)"),
       tr("Bölgeyi yakalar ve sadece diske kaydeder."),
       QStringLiteral("Meta+Print"), QStringLiteral("Meta+Print")});

  m_shortcuts.insert(
      ShortcutAction::CaptureFullscreenDelayed,
      {ShortcutAction::CaptureFullscreenDelayed,
       QStringLiteral("fullscreen_delayed"), tr("Gecikmeli Tam Ekran (5s)"),
       tr("5 saniye sonra tam ekran yakalar."),
       QStringLiteral("Ctrl+Alt+Print"), QStringLiteral("Ctrl+Alt+Print")});

  m_shortcuts.insert(
      ShortcutAction::CaptureWindowDelayed,
      {ShortcutAction::CaptureWindowDelayed, QStringLiteral("window_delayed"),
       tr("Gecikmeli Pencere (5s)"),
       tr("5 saniye sonra aktif pencereyi yakalar."),
       QStringLiteral("Alt+Shift+Print"), QStringLiteral("Alt+Shift+Print")});

  m_shortcuts.insert(
      ShortcutAction::OpenGallery,
      {ShortcutAction::OpenGallery, QStringLiteral("gallery"),
       tr("Galeriyi Aç"), tr("Ekran görüntüsü galeri ve kütüphanesini açar."),
       QStringLiteral("Meta+Shift+G"), QStringLiteral("Meta+Shift+G")});

  m_shortcuts.insert(
      ShortcutAction::OpenSettings,
      {ShortcutAction::OpenSettings, QStringLiteral("settings"),
       tr("Ayarları Aç"), tr("Uygulama ayarları penceresini açar."),
       QStringLiteral("Meta+Shift+S"), QStringLiteral("Meta+Shift+S")});

  m_shortcuts.insert(
      ShortcutAction::ColorPicker,
      {ShortcutAction::ColorPicker, QStringLiteral("color_picker"),
       tr("Renk Damlalığı (Color Picker)"),
       tr("Ekranda piksel renk kodunu kopyalar."),
       QStringLiteral("Meta+Shift+C"), QStringLiteral("Meta+Shift+C")});

  m_shortcuts.insert(
      ShortcutAction::ToggleRecording,
      {ShortcutAction::ToggleRecording, QStringLiteral("record"),
       tr("Ekran Kaydı Başlat/Durdur"),
       tr("Video veya GIF ekran kaydını başlatır ya da durdurur."),
       QStringLiteral("Ctrl+Shift+R"), QStringLiteral("Ctrl+Shift+R")});
}

void ShortcutManager::load() {
  m_settings.beginGroup(SHORTCUT_GROUP);
  for (auto it = m_shortcuts.begin(); it != m_shortcuts.end(); ++it) {
    const QString key = it.value().id;
    if (m_settings.contains(key)) {
      it.value().currentKeySequence = m_settings.value(key).toString();
    }
  }
  m_settings.endGroup();
}

void ShortcutManager::save() {
  m_settings.beginGroup(SHORTCUT_GROUP);
  for (auto it = m_shortcuts.constBegin(); it != m_shortcuts.constEnd(); ++it) {
    m_settings.setValue(it.value().id, it.value().currentKeySequence);
  }
  m_settings.endGroup();
  m_settings.sync();
}

QVector<ShortcutItem> ShortcutManager::shortcuts() const {
  QVector<ShortcutItem> list;
  for (auto it = m_shortcuts.constBegin(); it != m_shortcuts.constEnd(); ++it) {
    list.append(it.value());
  }
  return list;
}

ShortcutItem ShortcutManager::shortcut(ShortcutAction action) const {
  return m_shortcuts.value(action);
}

QString ShortcutManager::keySequence(ShortcutAction action) const {
  return m_shortcuts.value(action).currentKeySequence;
}

bool ShortcutManager::setKeySequence(ShortcutAction action,
                                     const QString &keySequence) {
  if (!m_shortcuts.contains(action)) {
    return false;
  }
  m_shortcuts[action].currentKeySequence = keySequence;
  save();
  emit shortcutsChanged();
  return true;
}

bool ShortcutManager::hasConflict(ShortcutAction action,
                                  const QString &candidateSequence,
                                  ShortcutAction *conflictingAction) const {
  if (candidateSequence.isEmpty()) {
    return false;
  }

  const QKeySequence candidate(candidateSequence);
  for (auto it = m_shortcuts.constBegin(); it != m_shortcuts.constEnd(); ++it) {
    if (it.key() == action) {
      continue;
    }
    const QKeySequence current(it.value().currentKeySequence);
    if (!current.isEmpty() && current == candidate) {
      if (conflictingAction) {
        *conflictingAction = it.key();
      }
      return true;
    }
  }
  return false;
}

QVector<QPair<ShortcutAction, ShortcutAction>>
ShortcutManager::allConflicts() const {
  QVector<QPair<ShortcutAction, ShortcutAction>> conflicts;
  const auto keys = m_shortcuts.keys();
  for (int i = 0; i < keys.size(); ++i) {
    const auto seqA = QKeySequence(m_shortcuts[keys[i]].currentKeySequence);
    if (seqA.isEmpty())
      continue;
    for (int j = i + 1; j < keys.size(); ++j) {
      const auto seqB = QKeySequence(m_shortcuts[keys[j]].currentKeySequence);
      if (seqA == seqB) {
        conflicts.append(qMakePair(keys[i], keys[j]));
      }
    }
  }
  return conflicts;
}

void ShortcutManager::resetToDefaults() {
  for (auto it = m_shortcuts.begin(); it != m_shortcuts.end(); ++it) {
    it.value().currentKeySequence = it.value().defaultKeySequence;
  }
  save();
  emit shortcutsChanged();
}

void ShortcutManager::resetShortcut(ShortcutAction action) {
  if (m_shortcuts.contains(action)) {
    m_shortcuts[action].currentKeySequence =
        m_shortcuts[action].defaultKeySequence;
    save();
    emit shortcutsChanged();
  }
}

QString ShortcutManager::generateKdeShortcutsConfig() const {
  QString out;
  out += QStringLiteral("# KDE kglobalshortcutsrc snippet for ro-ScreenShot\n");
  out += QStringLiteral("[org.ro_asd.ro_screenshot.desktop]\n");
  out += QStringLiteral("_k_friendly_name=ro-ScreenShot\n");

  const auto mapActionToCli = [](ShortcutAction action) -> QString {
    switch (action) {
    case ShortcutAction::CaptureRegion:
      return QStringLiteral("ro-screenshot --region");
    case ShortcutAction::CaptureFullscreen:
      return QStringLiteral("ro-screenshot --fullscreen");
    case ShortcutAction::CaptureWindow:
      return QStringLiteral("ro-screenshot --window");
    case ShortcutAction::CaptureLastRegion:
      return QStringLiteral("ro-screenshot --last-region");
    case ShortcutAction::CaptureRegionCopyOnly:
      return QStringLiteral("ro-screenshot --region --copy-only");
    case ShortcutAction::CaptureRegionSaveOnly:
      return QStringLiteral("ro-screenshot --region --save-only");
    case ShortcutAction::CaptureFullscreenDelayed:
      return QStringLiteral("ro-screenshot --fullscreen --delay 5");
    case ShortcutAction::CaptureWindowDelayed:
      return QStringLiteral("ro-screenshot --window --delay 5");
    case ShortcutAction::OpenGallery:
      return QStringLiteral("ro-screenshot --library");
    case ShortcutAction::OpenSettings:
      return QStringLiteral("ro-screenshot --settings");
    case ShortcutAction::ColorPicker:
      return QStringLiteral("ro-screenshot --color-picker");
    case ShortcutAction::ToggleRecording:
      return QStringLiteral("ro-screenshot --record");
    }
    return QStringLiteral("ro-screenshot");
  };

  for (auto it = m_shortcuts.constBegin(); it != m_shortcuts.constEnd(); ++it) {
    out += QStringLiteral("%1=%2,%3,%4\n")
               .arg(it.value().id, it.value().currentKeySequence,
                    it.value().defaultKeySequence, it.value().name);
    out += QStringLiteral("# Command: %1\n").arg(mapActionToCli(it.key()));
  }
  return out;
}

QString ShortcutManager::generateGnomeShortcutsScript() const {
  QString out;
  out += QStringLiteral("#!/usr/bin/env bash\n");
  out += QStringLiteral(
      "# GNOME Custom Shortcut Configurator for ro-ScreenShot\n\n");
  out += QStringLiteral("BASE_PATH=\"/org/gnome/settings-daemon/plugins/"
                        "media-keys/custom-keybindings\"\n");

  int index = 0;
  QStringList customList;

  const auto mapActionToCli = [](ShortcutAction action) -> QString {
    switch (action) {
    case ShortcutAction::CaptureRegion:
      return QStringLiteral("ro-screenshot --region");
    case ShortcutAction::CaptureFullscreen:
      return QStringLiteral("ro-screenshot --fullscreen");
    case ShortcutAction::CaptureWindow:
      return QStringLiteral("ro-screenshot --window");
    case ShortcutAction::CaptureLastRegion:
      return QStringLiteral("ro-screenshot --last-region");
    case ShortcutAction::CaptureRegionCopyOnly:
      return QStringLiteral("ro-screenshot --region --copy-only");
    case ShortcutAction::CaptureRegionSaveOnly:
      return QStringLiteral("ro-screenshot --region --save-only");
    case ShortcutAction::CaptureFullscreenDelayed:
      return QStringLiteral("ro-screenshot --fullscreen --delay 5");
    case ShortcutAction::CaptureWindowDelayed:
      return QStringLiteral("ro-screenshot --window --delay 5");
    case ShortcutAction::OpenGallery:
      return QStringLiteral("ro-screenshot --library");
    case ShortcutAction::OpenSettings:
      return QStringLiteral("ro-screenshot --settings");
    case ShortcutAction::ColorPicker:
      return QStringLiteral("ro-screenshot --color-picker");
    case ShortcutAction::ToggleRecording:
      return QStringLiteral("ro-screenshot --record");
    }
    return QStringLiteral("ro-screenshot");
  };

  for (auto it = m_shortcuts.constBegin(); it != m_shortcuts.constEnd();
       ++it, ++index) {
    const QString customBinding = QStringLiteral("custom%1").arg(index);
    customList.append(
        QStringLiteral("'%1/%2/'")
            .arg(QStringLiteral("/org/gnome/settings-daemon/plugins/media-keys/"
                                "custom-keybindings"),
                 customBinding));

    out += QStringLiteral("gsettings set "
                          "org.gnome.settings-daemon.plugins.media-keys.custom-"
                          "keybinding:$BASE_PATH/%1/ name \"%2\"\n")
               .arg(customBinding, it.value().name);
    out += QStringLiteral("gsettings set "
                          "org.gnome.settings-daemon.plugins.media-keys.custom-"
                          "keybinding:$BASE_PATH/%1/ command \"%2\"\n")
               .arg(customBinding, mapActionToCli(it.key()));
    out += QStringLiteral("gsettings set "
                          "org.gnome.settings-daemon.plugins.media-keys.custom-"
                          "keybinding:$BASE_PATH/%1/ binding \"%2\"\n\n")
               .arg(customBinding, it.value().currentKeySequence);
  }

  out += QStringLiteral(
             "gsettings set org.gnome.settings-daemon.plugins.media-keys "
             "custom-keybindings \"[%1]\"\n")
             .arg(customList.join(QStringLiteral(", ")));
  return out;
}

QString ShortcutManager::generateHyprlandConfig() const {
  QString out;
  out += QStringLiteral("# Hyprland Keybindings for ro-ScreenShot\n");
  out +=
      QStringLiteral("# Add these lines to ~/.config/hypr/hyprland.conf\n\n");

  const auto mapQtToHypr = [](const QString &seq) -> QString {
    QString res = seq;
    res.replace("Ctrl", "CONTROL");
    res.replace("Shift", "SHIFT");
    res.replace("Alt", "ALT");
    res.replace("Meta", "SUPER");
    res.replace("+", ", ");
    return res;
  };

  for (auto it = m_shortcuts.constBegin(); it != m_shortcuts.constEnd(); ++it) {
    QString cli;
    switch (it.key()) {
    case ShortcutAction::CaptureRegion:
      cli = "ro-screenshot --region";
      break;
    case ShortcutAction::CaptureFullscreen:
      cli = "ro-screenshot --fullscreen";
      break;
    case ShortcutAction::CaptureWindow:
      cli = "ro-screenshot --window";
      break;
    case ShortcutAction::CaptureLastRegion:
      cli = "ro-screenshot --last-region";
      break;
    case ShortcutAction::CaptureRegionCopyOnly:
      cli = "ro-screenshot --region --copy-only";
      break;
    case ShortcutAction::CaptureRegionSaveOnly:
      cli = "ro-screenshot --region --save-only";
      break;
    case ShortcutAction::CaptureFullscreenDelayed:
      cli = "ro-screenshot --fullscreen --delay 5";
      break;
    case ShortcutAction::CaptureWindowDelayed:
      cli = "ro-screenshot --window --delay 5";
      break;
    case ShortcutAction::OpenGallery:
      cli = "ro-screenshot --library";
      break;
    case ShortcutAction::OpenSettings:
      cli = "ro-screenshot --settings";
      break;
    case ShortcutAction::ColorPicker:
      cli = "ro-screenshot --color-picker";
      break;
    case ShortcutAction::ToggleRecording:
      cli = "ro-screenshot --record";
      break;
    }
    out += QStringLiteral("bind = %1, exec, %2\n")
               .arg(mapQtToHypr(it.value().currentKeySequence), cli);
  }
  return out;
}

QString ShortcutManager::generateSwayConfig() const {
  QString out;
  out += QStringLiteral("# Sway Keybindings for ro-ScreenShot\n");
  out += QStringLiteral("# Add these lines to ~/.config/sway/config\n\n");

  const auto mapQtToSway = [](const QString &seq) -> QString {
    QString res = seq;
    res.replace("Meta", "Mod4");
    res.replace("Alt", "Mod1");
    return res;
  };

  for (auto it = m_shortcuts.constBegin(); it != m_shortcuts.constEnd(); ++it) {
    QString cli;
    switch (it.key()) {
    case ShortcutAction::CaptureRegion:
      cli = "ro-screenshot --region";
      break;
    case ShortcutAction::CaptureFullscreen:
      cli = "ro-screenshot --fullscreen";
      break;
    case ShortcutAction::CaptureWindow:
      cli = "ro-screenshot --window";
      break;
    case ShortcutAction::CaptureLastRegion:
      cli = "ro-screenshot --last-region";
      break;
    case ShortcutAction::CaptureRegionCopyOnly:
      cli = "ro-screenshot --region --copy-only";
      break;
    case ShortcutAction::CaptureRegionSaveOnly:
      cli = "ro-screenshot --region --save-only";
      break;
    case ShortcutAction::CaptureFullscreenDelayed:
      cli = "ro-screenshot --fullscreen --delay 5";
      break;
    case ShortcutAction::CaptureWindowDelayed:
      cli = "ro-screenshot --window --delay 5";
      break;
    case ShortcutAction::OpenGallery:
      cli = "ro-screenshot --library";
      break;
    case ShortcutAction::OpenSettings:
      cli = "ro-screenshot --settings";
      break;
    case ShortcutAction::ColorPicker:
      cli = "ro-screenshot --color-picker";
      break;
    case ShortcutAction::ToggleRecording:
      cli = "ro-screenshot --record";
      break;
    }
    out += QStringLiteral("bindsym %1 exec %2\n")
               .arg(mapQtToSway(it.value().currentKeySequence), cli);
  }
  return out;
}

} // namespace ro_screenshot
