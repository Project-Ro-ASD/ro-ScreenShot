#pragma once

#include <QDBusConnection>
#include <QObject>
#include <QString>

namespace ro_screenshot {

class SystemTrayManager : public QObject {
  Q_OBJECT

public:
  explicit SystemTrayManager(QObject *parent = nullptr);
  ~SystemTrayManager() override;

  bool initTray();
  void hideTray();
  bool isTrayActive() const;

signals:
  void captureRegionRequested();
  void captureFullscreenRequested();
  void captureWindowRequested();
  void openLibraryRequested();
  void openSettingsRequested();
  void quitRequested();

private:
  bool m_isActive{false};
};

} // namespace ro_screenshot
