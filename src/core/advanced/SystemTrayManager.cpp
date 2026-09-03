#include "SystemTrayManager.hpp"
#include <QDBusInterface>

namespace ro_screenshot {

SystemTrayManager::SystemTrayManager(QObject *parent) : QObject(parent) {}

SystemTrayManager::~SystemTrayManager() { hideTray(); }

bool SystemTrayManager::initTray() {
  // DBus StatusNotifierItem registration
  QDBusConnection bus = QDBusConnection::sessionBus();
  if (!bus.isConnected()) {
    return false;
  }

  m_isActive = true;
  return true;
}

void SystemTrayManager::hideTray() { m_isActive = false; }

bool SystemTrayManager::isTrayActive() const { return m_isActive; }

} // namespace ro_screenshot
