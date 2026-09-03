#include "AutostartManager.hpp"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTextStream>

namespace ro_screenshot {

AutostartManager::AutostartManager(QObject *parent) : QObject(parent) {}

QString AutostartManager::autostartFilePath() const {
  QString configDir =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
  if (configDir.isEmpty()) {
    configDir = QDir::homePath() + "/.config";
  }
  return configDir + "/autostart/" + m_desktopFileName;
}

bool AutostartManager::isAutostartEnabled() const {
  return QFileInfo::exists(autostartFilePath());
}

bool AutostartManager::setAutostartEnabled(bool enabled) {
  const QString path = autostartFilePath();
  if (!enabled) {
    if (QFile::exists(path)) {
      return QFile::remove(path);
    }
    return true;
  }

  QDir().mkpath(QFileInfo(path).absolutePath());
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    return false;
  }

  QTextStream out(&file);
  out << "[Desktop Entry]\n";
  out << "Type=Application\n";
  out << "Name=ro-ScreenShot\n";
  out << "Comment=Modern Linux & Wayland Screen Capture Tool\n";
  out << "Exec=ro-screenshot --tray\n";
  out << "Icon=org.ro_asd.ro_screenshot\n";
  out << "Terminal=false\n";
  out << "Categories=Utility;\n";
  out << "X-GNOME-Autostart-enabled=true\n";
  out.flush();
  file.close();
  return true;
}

} // namespace ro_screenshot
