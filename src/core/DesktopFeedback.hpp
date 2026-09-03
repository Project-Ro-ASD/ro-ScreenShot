#pragma once

#include "core/Types.hpp"
#include <QColor>
#include <QDBusConnection>
#include <QObject>
#include <QSoundEffect>
#include <QString>
#include <QStringList>

namespace ro_screenshot {

class DesktopFeedback final : public QObject {
  Q_OBJECT

public:
  explicit DesktopFeedback(QObject *parent = nullptr);
  ~DesktopFeedback() override = default;

  void showCaptureSuccess(const QString &filePath, const QString &fileName,
                          bool savedToDisk, bool copiedToClipboard);
  void showError(const QString &message,
                 CaptureErrorCode code = CaptureErrorCode::Unknown);
  void showProgress(int percent, const QString &statusText);
  void playShutter();

  // DND (Do Not Disturb) inspection
  bool isDoNotDisturbActive() const;

  // Color formatters
  static ColorSample sampleColor(const QColor &color);
  static QString formatColor(const QColor &color, ColorFormat format);

signals:
  void notificationActionTriggered(const QString &actionKey,
                                   const QString &filePath);
  void openFileRequested(const QString &filePath);
  void openFolderRequested(const QString &filePath);
  void copyImageRequested(const QString &filePath);

private slots:
  void handleNotificationAction(uint id, const QString &actionKey);

private:
  void notify(const QString &summary, const QString &body,
              const QStringList &actions = {},
              const QString &targetFilePath = {});
  void setupShutterSound();

  QSoundEffect m_shutterSound;
  uint m_lastNotificationId{0};
  QString m_lastNotificationFilePath;
};

} // namespace ro_screenshot
