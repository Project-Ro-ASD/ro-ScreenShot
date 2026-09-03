#pragma once

#include <QObject>
#include <QSoundEffect>
#include <QString>

namespace ro_screenshot {

class DesktopFeedback final : public QObject {
  Q_OBJECT

public:
  explicit DesktopFeedback(QObject *parent = nullptr);

  void showCaptureSuccess(const QString &filePath, const QString &fileName,
                          bool savedToDisk, bool copiedToClipboard);
  void showError(const QString &message);
  void playShutter();

private:
  void notify(const QString &summary, const QString &body,
              const QStringList &actions = {});

  QSoundEffect m_shutterSound;
};

} // namespace ro_screenshot
