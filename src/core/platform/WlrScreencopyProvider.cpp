#include "WlrScreencopyProvider.hpp"
#include <QGuiApplication>
#include <QProcess>
#include <QScreen>
#include <QStandardPaths>
#include <QTimer>

namespace ro_screenshot {

WlrScreencopyProvider::WlrScreencopyProvider(QObject *parent)
    : IScreenshotProvider(parent) {}

bool WlrScreencopyProvider::isAvailable() const {
  return !QStandardPaths::findExecutable(QStringLiteral("grim")).isEmpty();
}

CaptureErrorCode
WlrScreencopyProvider::classifyGrimError(const QString &standardError) {
  const QString normalized = standardError.trimmed().toLower();
  if (normalized.contains(
          QStringLiteral("doesn't support the screen capture protocol")) ||
      normalized.contains(
          QStringLiteral("does not support the screen capture protocol")) ||
      normalized.contains(QStringLiteral("screencopy protocol")) ||
      normalized.contains(QStringLiteral("unsupported protocol"))) {
    return CaptureErrorCode::PortalUnavailable;
  }
  if (normalized.contains(QStringLiteral("permission denied")) ||
      normalized.contains(QStringLiteral("access denied")) ||
      normalized.contains(QStringLiteral("not authorized"))) {
    return CaptureErrorCode::PermissionDenied;
  }
  return CaptureErrorCode::Unknown;
}

void WlrScreencopyProvider::cancel() {
  if (!m_isCapturing) {
    return;
  }

  m_isCapturing = false;
  if (m_timeoutTimer) {
    m_timeoutTimer->stop();
    m_timeoutTimer->deleteLater();
    m_timeoutTimer = nullptr;
  }
  if (m_process) {
    m_process->disconnect(this);
    m_process->kill();
    m_process->deleteLater();
    m_process = nullptr;
  }
  emit captureCancelled();
}

void WlrScreencopyProvider::capture(CaptureMode mode,
                                    const QVariantMap & /*options*/) {
  cancel();
  m_isCapturing = true;
  emit captureStarted(mode);

  auto *process = new QProcess(this);
  auto *timeout = new QTimer(this);
  timeout->setSingleShot(true);
  m_process = process;
  m_timeoutTimer = timeout;

  connect(timeout, &QTimer::timeout, this, [this, process, timeout]() {
    if (!m_isCapturing || process != m_process) {
      return;
    }
    m_isCapturing = false;
    m_process = nullptr;
    m_timeoutTimer = nullptr;
    process->kill();
    process->deleteLater();
    timeout->deleteLater();
    emit captureFailed(QStringLiteral("grim capture timed out."),
                       CaptureErrorCode::Timeout);
  });

  connect(process, &QProcess::errorOccurred, this,
          [this, process, timeout](QProcess::ProcessError error) {
            if (error != QProcess::FailedToStart || !m_isCapturing ||
                process != m_process) {
              return;
            }
            m_isCapturing = false;
            m_process = nullptr;
            m_timeoutTimer = nullptr;
            timeout->stop();
            process->deleteLater();
            timeout->deleteLater();
            emit captureFailed(
                QStringLiteral("grim process could not be started."),
                CaptureErrorCode::PortalUnavailable);
          });

  connect(process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
          this,
          [this, process, timeout](int exitCode, QProcess::ExitStatus status) {
            if (!m_isCapturing || process != m_process) {
              return;
            }
            m_isCapturing = false;
            m_process = nullptr;
            m_timeoutTimer = nullptr;
            timeout->stop();

            const QByteArray stderrOutput = process->readAllStandardError();
            const QByteArray imageData = process->readAllStandardOutput();
            process->deleteLater();
            timeout->deleteLater();

            if (status != QProcess::NormalExit || exitCode != 0) {
              const QString error = QString::fromUtf8(stderrOutput).trimmed();
              emit captureFailed(
                  tr("grim ekran yakalama işlemi başarısız oldu: %1")
                      .arg(error.isEmpty() ? tr("Bilinmeyen hata") : error),
                  classifyGrimError(error));
              return;
            }

            QImage image;
            if (!image.loadFromData(imageData, "PNG") || image.isNull()) {
              emit captureFailed(
                  QStringLiteral("Invalid image stream returned by grim."),
                  CaptureErrorCode::InvalidImage);
              return;
            }
            emit captureReady(image, image.rect());
          });

  process->start(
      QStringLiteral("grim"),
      {QStringLiteral("-t"), QStringLiteral("png"), QStringLiteral("-")});
  timeout->start(6000);
}

} // namespace ro_screenshot
