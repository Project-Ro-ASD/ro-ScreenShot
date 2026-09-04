#include "DesktopFeedback.hpp"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusPendingReply>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QUrl>

namespace ro_screenshot {

namespace {
constexpr auto notificationsService = "org.freedesktop.Notifications";
constexpr auto notificationsPath = "/org/freedesktop/Notifications";
constexpr auto notificationsInterface = "org.freedesktop.Notifications";
} // namespace

DesktopFeedback::DesktopFeedback(QObject *parent) : QObject(parent) {
  setupShutterSound();

  // Connect to Notification ActionInvoked signal
  QDBusConnection::sessionBus().connect(
      QString::fromLatin1(notificationsService),
      QString::fromLatin1(notificationsPath),
      QString::fromLatin1(notificationsInterface),
      QStringLiteral("ActionInvoked"), this,
      SLOT(handleNotificationAction(uint, QString)));
}

void DesktopFeedback::setupShutterSound() {
  // QSoundEffect in Qt 6 requires uncompressed PCM WAV files
  const QStringList candidatePaths = {
      QStringLiteral("/usr/share/sounds/freedesktop/stereo/camera-shutter.wav"),
      QStringLiteral(
          "/usr/share/sounds/gnome/default/alerts/camera-shutter.wav"),
      QStringLiteral("/usr/share/sounds/oxygen/stereo/camera-shutter.wav")};

  for (const QString &path : candidatePaths) {
    if (QFile::exists(path)) {
      m_shutterSound.setSource(QUrl::fromLocalFile(path));
      m_shutterSound.setVolume(0.45F);
      break;
    }
  }
}

bool DesktopFeedback::isDoNotDisturbActive() const {
  QDBusInterface notificationService(
      QString::fromLatin1(notificationsService),
      QString::fromLatin1(notificationsPath),
      QString::fromLatin1(notificationsInterface),
      QDBusConnection::sessionBus());
  if (notificationService.isValid()) {
    const QVariant inhibited = notificationService.property("Inhibited");
    if (inhibited.isValid() && inhibited.toBool()) {
      return true;
    }
  }
  return false;
}

void DesktopFeedback::showCaptureSuccess(const QString &filePath,
                                         const QString &fileName,
                                         bool savedToDisk,
                                         bool copiedToClipboard) {
  if (isDoNotDisturbActive()) {
    return;
  }

  QStringList status;
  if (savedToDisk) {
    status << tr("Diske kaydedildi: %1").arg(fileName);
  }
  if (copiedToClipboard) {
    status << tr("Panoya kopyalandı");
  }

  QStringList actions;
  if (savedToDisk && !filePath.isEmpty()) {
    actions << QStringLiteral("open_folder") << tr("Klasörü Aç");
    actions << QStringLiteral("open_file") << tr("Görüntüle");
  }
  if (!copiedToClipboard && !filePath.isEmpty()) {
    actions << QStringLiteral("copy") << tr("Panoya Kopyala");
  }

  notify(tr("Ekran Görüntüsü Yakalandı"), status.join(QStringLiteral(" • ")),
         actions, filePath);
}

void DesktopFeedback::showError(const QString &message,
                                CaptureErrorCode /*code*/) {
  if (isDoNotDisturbActive()) {
    return;
  }
  notify(tr("Ekran Görüntüsü Alınamadı"), message);
}

void DesktopFeedback::showProgress(int percent, const QString &statusText) {
  if (isDoNotDisturbActive()) {
    return;
  }
  notify(tr("İşleniyor (%1%)").arg(percent), statusText);
}

void DesktopFeedback::playShutter() {
  if (isDoNotDisturbActive()) {
    return;
  }
  if (m_shutterSound.source().isLocalFile() &&
      m_shutterSound.status() != QSoundEffect::Error) {
    m_shutterSound.play();
    return;
  }
  // Native freedesktop sound event fallback for modern Linux desktops
  QProcess::startDetached(
      QStringLiteral("canberra-gtk-play"),
      {QStringLiteral("-i"), QStringLiteral("screen-capture"),
       QStringLiteral("-d"), QStringLiteral("ro-ScreenShot")});
}

void DesktopFeedback::notify(const QString &summary, const QString &body,
                             const QStringList &actions,
                             const QString &targetFilePath) {
  QDBusInterface notificationService(
      QString::fromLatin1(notificationsService),
      QString::fromLatin1(notificationsPath),
      QString::fromLatin1(notificationsInterface),
      QDBusConnection::sessionBus());
  if (!notificationService.isValid()) {
    return;
  }

  m_lastNotificationFilePath = targetFilePath;

  QVariantMap hints;
  hints.insert(QStringLiteral("desktop-entry"),
               QStringLiteral("org.ro_asd.ro_screenshot"));
  if (!targetFilePath.isEmpty()) {
    hints.insert(QStringLiteral("image-path"), targetFilePath);
  }

  notificationService.asyncCall(
      QStringLiteral("Notify"), QStringLiteral("ro-ScreenShot"), uint{0},
      QStringLiteral("camera-photo"), summary, body, actions, hints, 5000);
}

void DesktopFeedback::handleNotificationAction(uint /*id*/,
                                               const QString &actionKey) {
  emit notificationActionTriggered(actionKey, m_lastNotificationFilePath);

  if (actionKey == QStringLiteral("open_folder")) {
    if (!m_lastNotificationFilePath.isEmpty()) {
      const QFileInfo fi(m_lastNotificationFilePath);
      QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absolutePath()));
    }
  } else if (actionKey == QStringLiteral("open_file")) {
    if (!m_lastNotificationFilePath.isEmpty()) {
      QDesktopServices::openUrl(
          QUrl::fromLocalFile(m_lastNotificationFilePath));
    }
  } else if (actionKey == QStringLiteral("copy")) {
    emit copyImageRequested(m_lastNotificationFilePath);
  }
}

ColorSample DesktopFeedback::sampleColor(const QColor &color) {
  ColorSample sample;
  sample.hex = color.name(QColor::HexRgb).toUpper();
  sample.rgb = QStringLiteral("rgb(%1, %2, %3)")
                   .arg(color.red())
                   .arg(color.green())
                   .arg(color.blue());
  sample.hsl = QStringLiteral("hsl(%1, %2%, %3%)")
                   .arg(std::max(0, color.hslHue()))
                   .arg(static_cast<int>(color.hslSaturationF() * 100))
                   .arg(static_cast<int>(color.lightnessF() * 100));
  sample.red = color.red();
  sample.green = color.green();
  sample.blue = color.blue();
  sample.alpha = color.alpha();
  return sample;
}

QString DesktopFeedback::formatColor(const QColor &color, ColorFormat format) {
  switch (format) {
  case ColorFormat::Hex:
    return color.name(QColor::HexRgb).toUpper();
  case ColorFormat::Rgb:
    return QStringLiteral("rgb(%1, %2, %3)")
        .arg(color.red())
        .arg(color.green())
        .arg(color.blue());
  case ColorFormat::Hsl:
    return QStringLiteral("hsl(%1, %2%, %3%)")
        .arg(std::max(0, color.hslHue()))
        .arg(static_cast<int>(color.hslSaturationF() * 100))
        .arg(static_cast<int>(color.lightnessF() * 100));
  }
  return color.name(QColor::HexRgb).toUpper();
}

} // namespace ro_screenshot
