#pragma once

#include <QDateTime>
#include <QMap>
#include <QObject>
#include <QString>
#include <QTimer>

namespace ro_screenshot {

struct EphemeralRecord {
  QString filePath;
  QDateTime expireAt;
  int ttlSeconds{3600};
};

class EphemeralShotManager : public QObject {
  Q_OBJECT

  Q_PROPERTY(
      int activeEphemeralCount READ activeEphemeralCount NOTIFY recordsChanged)

public:
  explicit EphemeralShotManager(QObject *parent = nullptr);
  ~EphemeralShotManager() override = default;

  int activeEphemeralCount() const;

  Q_INVOKABLE void registerEphemeralFile(const QString &filePath,
                                         int ttlSeconds = 3600);
  Q_INVOKABLE void cancelEphemeral(const QString &filePath);
  Q_INVOKABLE bool isEphemeral(const QString &filePath) const;
  Q_INVOKABLE int remainingSeconds(const QString &filePath) const;
  Q_INVOKABLE void checkAndPurgeExpired();

signals:
  void recordsChanged();
  void filePurged(const QString &filePath);

private slots:
  void handlePurgeTick();

private:
  QMap<QString, EphemeralRecord> m_records;
  QTimer *m_purgeTimer{nullptr};
};

} // namespace ro_screenshot
