#pragma once

#include "IScreenshotProvider.hpp"
#include <QProcess>
#include <QTimer>

namespace ro_screenshot {

class WlrScreencopyProvider : public IScreenshotProvider {
  Q_OBJECT

public:
  explicit WlrScreencopyProvider(QObject *parent = nullptr);
  ~WlrScreencopyProvider() override = default;

  QString name() const override { return QStringLiteral("wlr-screencopy"); }
  bool isAvailable() const override;
  void capture(CaptureMode mode, const QVariantMap &options = {}) override;
  void cancel() override;

private:
  QProcess *m_process{nullptr};
  QTimer *m_timeoutTimer{nullptr};
  bool m_isCapturing{false};
};

} // namespace ro_screenshot
