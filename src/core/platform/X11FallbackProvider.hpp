#pragma once

#include "IScreenshotProvider.hpp"
#include "ScreenGeometryManager.hpp"
#include <QObject>

namespace ro_screenshot {

class X11FallbackProvider : public IScreenshotProvider {
  Q_OBJECT

public:
  explicit X11FallbackProvider(ScreenGeometryManager *geometryManager,
                               QObject *parent = nullptr);
  ~X11FallbackProvider() override = default;

  QString name() const override { return QStringLiteral("X11Fallback"); }
  bool isAvailable() const override;
  void capture(CaptureMode mode, const QVariantMap &options = {}) override;
  void cancel() override;

  QImage captureCombinedDesktop() const;
  QImage captureActiveScreen() const;
  QImage captureActiveWindow() const;

private:
  ScreenGeometryManager *m_geometryManager{nullptr};
  bool m_isCapturing{false};
};

} // namespace ro_screenshot
