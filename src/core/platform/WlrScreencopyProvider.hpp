#pragma once

#include "IScreenshotProvider.hpp"
#include <QProcess>

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
  bool executeGrim(const QStringList &args, const QRect &targetRect);
};

} // namespace ro_screenshot
