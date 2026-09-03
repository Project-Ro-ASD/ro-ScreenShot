#pragma once

#include "core/Types.hpp"
#include <QImage>
#include <QObject>
#include <QRect>
#include <QString>
#include <QVariantMap>

namespace ro_screenshot {

class IScreenshotProvider : public QObject {
  Q_OBJECT

public:
  explicit IScreenshotProvider(QObject *parent = nullptr) : QObject(parent) {}
  ~IScreenshotProvider() override = default;

  virtual QString name() const = 0;
  virtual bool isAvailable() const = 0;
  virtual void capture(CaptureMode mode, const QVariantMap &options = {}) = 0;
  virtual void cancel() = 0;

signals:
  void captureStarted(CaptureMode mode);
  void captureReady(const QImage &image, const QRect &sourceRect);
  void captureFailed(const QString &errorMessage, CaptureErrorCode errorCode);
  void captureCancelled();
};

} // namespace ro_screenshot
