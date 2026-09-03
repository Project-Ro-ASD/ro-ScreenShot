#include "DesktopFeedback.hpp"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QUrl>

namespace ro_screenshot {

namespace {
constexpr auto notificationsService = "org.freedesktop.Notifications";
constexpr auto notificationsPath = "/org/freedesktop/Notifications";
constexpr auto notificationsInterface = "org.freedesktop.Notifications";
} // namespace

DesktopFeedback::DesktopFeedback(QObject *parent) : QObject(parent) {
  m_shutterSound.setSource(QUrl::fromLocalFile(
      "/usr/share/sounds/freedesktop/stereo/camera-shutter.oga"));
  m_shutterSound.setVolume(0.45F);
}

void DesktopFeedback::showCaptureSuccess(const QString &filePath,
                                         const QString &fileName,
                                         bool savedToDisk,
                                         bool copiedToClipboard) {
  QStringList status;
  if (savedToDisk) {
    status << tr("Saved to %1").arg(fileName);
  }
  if (copiedToClipboard) {
    status << tr("Copied to clipboard");
  }
  Q_UNUSED(filePath)
  notify(tr("Screenshot captured"), status.join(QStringLiteral(" • ")));
}

void DesktopFeedback::showError(const QString &message) {
  notify(tr("Screenshot failed"), message);
}

void DesktopFeedback::playShutter() {
  if (m_shutterSound.source().isLocalFile()) {
    m_shutterSound.play();
  }
}

void DesktopFeedback::notify(const QString &summary, const QString &body,
                             const QStringList &actions) {
  QDBusInterface notificationService(
      QString::fromLatin1(notificationsService),
      QString::fromLatin1(notificationsPath),
      QString::fromLatin1(notificationsInterface),
      QDBusConnection::sessionBus());
  if (!notificationService.isValid()) {
    return;
  }

  QVariantMap hints;
  hints.insert(QStringLiteral("desktop-entry"),
               QStringLiteral("org.ro_asd.ro_screenshot"));
  notificationService.asyncCall(
      QStringLiteral("Notify"), QStringLiteral("Ro-ScreenShot"), uint{0},
      QStringLiteral("camera-photo"), summary, body, actions, hints, 5000);
}

} // namespace ro_screenshot
