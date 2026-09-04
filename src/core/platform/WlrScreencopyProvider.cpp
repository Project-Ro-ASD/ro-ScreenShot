#include "WlrScreencopyProvider.hpp"
#include <QBuffer>
#include <QGuiApplication>
#include <QProcess>
#include <QScreen>
#include <QStandardPaths>

namespace ro_screenshot {

WlrScreencopyProvider::WlrScreencopyProvider(QObject *parent)
    : IScreenshotProvider(parent) {}

bool WlrScreencopyProvider::isAvailable() const {
  return !QStandardPaths::findExecutable(QStringLiteral("grim")).isEmpty();
}

void WlrScreencopyProvider::cancel() { emit captureCancelled(); }

bool WlrScreencopyProvider::executeGrim(const QStringList &args,
                                        const QRect &targetRect) {
  QProcess process;
  process.start("grim", QStringList() << "-t" << "png" << args << "-");

  if (!process.waitForStarted(3000)) {
    emit captureFailed(QStringLiteral("grim process could not be started."),
                       CaptureErrorCode::PortalUnavailable);
    return false;
  }

  if (!process.waitForFinished(6000)) {
    process.kill();
    emit captureFailed(QStringLiteral("grim process timed out."),
                       CaptureErrorCode::Timeout);
    return false;
  }

  if (process.exitCode() != 0) {
    emit captureFailed(QStringLiteral("grim capture failed: ") +
                           QString::fromUtf8(process.readAllStandardError()),
                       CaptureErrorCode::Unknown);
    return false;
  }

  QByteArray data = process.readAllStandardOutput();
  QImage img;
  if (!img.loadFromData(data, "PNG") || img.isNull()) {
    emit captureFailed(QStringLiteral("Invalid image stream returned by grim."),
                       CaptureErrorCode::InvalidImage);
    return false;
  }

  emit captureReady(img, targetRect.isValid() ? targetRect : img.rect());
  return true;
}

void WlrScreencopyProvider::capture(CaptureMode mode,
                                    const QVariantMap & /*options*/) {
  emit captureStarted(mode);
  executeGrim({}, QRect());
}

} // namespace ro_screenshot
