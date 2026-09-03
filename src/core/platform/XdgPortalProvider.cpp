#include "XdgPortalProvider.hpp"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusPendingReply>
#include <QFileInfo>
#include <QGuiApplication>
#include <QUrl>
#include <QUuid>

namespace ro_screenshot {

namespace {
constexpr auto portalService = "org.freedesktop.portal.Desktop";
constexpr auto portalPath = "/org/freedesktop/portal/desktop";
constexpr auto portalInterface = "org.freedesktop.portal.Screenshot";
constexpr auto requestInterface = "org.freedesktop.portal.Request";
} // namespace

XdgPortalProvider::XdgPortalProvider(QObject *parent)
    : IScreenshotProvider(parent), m_timeoutTimer(new QTimer(this)) {
  m_timeoutTimer->setSingleShot(true);
  connect(m_timeoutTimer, &QTimer::timeout, this,
          &XdgPortalProvider::handleTimeout);
}

XdgPortalProvider::~XdgPortalProvider() { cancel(); }

bool XdgPortalProvider::isAvailable() const {
  if (!QDBusConnection::sessionBus().isConnected()) {
    return false;
  }
  QDBusInterface portal(
      QString::fromLatin1(portalService), QString::fromLatin1(portalPath),
      QString::fromLatin1(portalInterface), QDBusConnection::sessionBus());
  return portal.isValid();
}

void XdgPortalProvider::setParentWindowHandle(const QString &handle) {
  m_parentWindowHandle = handle;
}

QString XdgPortalProvider::parentWindowHandle() const {
  return m_parentWindowHandle;
}

void XdgPortalProvider::setTimeoutDurationMs(int ms) {
  m_timeoutDurationMs = ms;
}

void XdgPortalProvider::capture(CaptureMode mode,
                                const QVariantMap &userOptions) {
  if (m_isCapturing) {
    cancel();
  }

  QDBusInterface portal(
      QString::fromLatin1(portalService), QString::fromLatin1(portalPath),
      QString::fromLatin1(portalInterface), QDBusConnection::sessionBus());
  if (!portal.isValid()) {
    emit captureFailed(
        tr("XDG Desktop Portal ekran görüntüsü servisi mevcut değil."),
        CaptureErrorCode::PortalUnavailable);
    return;
  }

  m_isCapturing = true;
  m_currentMode = mode;
  emit captureStarted(mode);

  QVariantMap options = userOptions;
  const QString token = QStringLiteral("ro_screenshot_%1")
                            .arg(QUuid::createUuid().toString(QUuid::Id128));
  options.insert(QStringLiteral("handle_token"), token);

  const uint version = portal.property("version").toUInt();
  const uint availableTargets = portal.property("AvailableTargets").toUInt();
  if (version >= 3U && availableTargets != 0U) {
    constexpr uint screenTarget = 1U;
    constexpr uint windowTarget = 2U;
    constexpr uint activeWindowTarget = 8U;
    uint target = screenTarget;
    if (mode == CaptureMode::Window) {
      target = (availableTargets & activeWindowTarget) != 0U
                   ? activeWindowTarget
                   : windowTarget;
    }
    if ((availableTargets & target) != 0U) {
      options.insert(QStringLiteral("target"), target);
      options.insert(QStringLiteral("interactive"), false);
    } else {
      options.insert(QStringLiteral("interactive"),
                     mode == CaptureMode::Window);
    }
  } else {
    options.insert(QStringLiteral("interactive"), mode == CaptureMode::Window);
  }

  m_pendingPortalRequestPath = expectedPortalRequestPath(token);
  if (!connectPortalResponse(m_pendingPortalRequestPath)) {
    m_isCapturing = false;
    emit captureFailed(tr("Portal istek yanıt sinyaline bağlanılamadı."),
                       CaptureErrorCode::PortalUnavailable);
    return;
  }

  // Start timeout timer
  if (m_timeoutDurationMs > 0) {
    m_timeoutTimer->start(m_timeoutDurationMs);
  }

  QString parentWindow = m_parentWindowHandle;
  if (parentWindow.isEmpty()) {
    parentWindow = QString();
  }

  m_activeWatcher = new QDBusPendingCallWatcher(
      portal.asyncCall(QStringLiteral("Screenshot"), parentWindow, options),
      this);

  connect(m_activeWatcher, &QDBusPendingCallWatcher::finished, this,
          [this](QDBusPendingCallWatcher *watcher) {
            QDBusPendingReply<QDBusObjectPath> reply = *watcher;
            cleanupPendingCall();

            if (reply.isError()) {
              disconnectPortalResponse();
              m_timeoutTimer->stop();
              m_isCapturing = false;
              emit captureFailed(tr("Portal isteği başarısız oldu: %1")
                                     .arg(reply.error().message()),
                                 CaptureErrorCode::PortalUnavailable);
              return;
            }

            // Race guard: If fast portal already completed and emitted
            // Response, m_isCapturing is already false.
            if (!m_isCapturing) {
              return;
            }

            const QString returnedPath = reply.value().path();
            if (!returnedPath.isEmpty() &&
                returnedPath != m_pendingPortalRequestPath) {
              disconnectPortalResponse();
              m_pendingPortalRequestPath = returnedPath;
              if (!connectPortalResponse(m_pendingPortalRequestPath)) {
                m_timeoutTimer->stop();
                m_isCapturing = false;
                emit captureFailed(
                    tr("Güncellenmiş portal istek yoluna bağlanılamadı."),
                    CaptureErrorCode::PortalUnavailable);
              }
            }
          });
}

void XdgPortalProvider::cancel() {
  if (!m_isCapturing) {
    return;
  }

  m_timeoutTimer->stop();
  cleanupPendingCall();

  if (!m_pendingPortalRequestPath.isEmpty()) {
    // Send Close request to portal if supported
    QDBusMessage msg = QDBusMessage::createMethodCall(
        QString::fromLatin1(portalService), m_pendingPortalRequestPath,
        QString::fromLatin1(requestInterface), QStringLiteral("Close"));
    QDBusConnection::sessionBus().send(msg);
  }

  disconnectPortalResponse();
  m_isCapturing = false;
  emit captureCancelled();
}

void XdgPortalProvider::handleTimeout() {
  if (!m_isCapturing) {
    return;
  }
  disconnectPortalResponse();
  cleanupPendingCall();
  m_isCapturing = false;
  emit captureFailed(tr("Portal ekran yakalama isteği zaman aşımına uğradı."),
                     CaptureErrorCode::Timeout);
}

void XdgPortalProvider::cleanupPendingCall() {
  if (m_activeWatcher) {
    m_activeWatcher->deleteLater();
    m_activeWatcher = nullptr;
  }
}

QString
XdgPortalProvider::expectedPortalRequestPath(const QString &token) const {
  QString sender = QDBusConnection::sessionBus().baseService();
  sender.remove(QLatin1Char(':'));
  sender.replace(QLatin1Char('.'), QLatin1Char('_'));
  return QStringLiteral("/org/freedesktop/portal/desktop/request/%1/%2")
      .arg(sender, token);
}

bool XdgPortalProvider::connectPortalResponse(const QString &path) {
  return !path.isEmpty() &&
         QDBusConnection::sessionBus().connect(
             QString::fromLatin1(portalService), path,
             QString::fromLatin1(requestInterface), QStringLiteral("Response"),
             this, SLOT(handlePortalResponse(uint, QVariantMap)));
}

void XdgPortalProvider::disconnectPortalResponse() {
  if (m_pendingPortalRequestPath.isEmpty()) {
    return;
  }
  QDBusConnection::sessionBus().disconnect(
      QString::fromLatin1(portalService), m_pendingPortalRequestPath,
      QString::fromLatin1(requestInterface), QStringLiteral("Response"), this,
      SLOT(handlePortalResponse(uint, QVariantMap)));
  m_pendingPortalRequestPath.clear();
}

void XdgPortalProvider::handlePortalResponse(uint response,
                                             const QVariantMap &results) {
  m_timeoutTimer->stop();
  disconnectPortalResponse();
  cleanupPendingCall();
  m_isCapturing = false;

  if (response != 0U) {
    if (response == 1U) {
      emit captureCancelled();
    } else {
      emit captureFailed(
          tr("Portal isteği reddetti veya bir hata oluştu (Kod: %1).")
              .arg(response),
          CaptureErrorCode::PortalRejected);
    }
    return;
  }

  const QString uri = results.value(QStringLiteral("uri")).toString();
  const QString localPath = QUrl(uri).toLocalFile();
  if (localPath.isEmpty() || !QFileInfo::exists(localPath)) {
    emit captureFailed(
        tr("Portal tarafından dönülen görsel dosyası bulunamadı."),
        CaptureErrorCode::InvalidImage);
    return;
  }

  const QImage image(localPath);
  if (image.isNull()) {
    emit captureFailed(
        tr("Portal tarafından geçersiz görsel verisi döndürüldü."),
        CaptureErrorCode::InvalidImage);
    return;
  }

  emit captureReady(image, image.rect());
}

} // namespace ro_screenshot
