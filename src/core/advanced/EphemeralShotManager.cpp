#include "EphemeralShotManager.hpp"
#include <QFile>
#include <QFileInfo>

namespace ro_screenshot {

EphemeralShotManager::EphemeralShotManager(QObject *parent) : QObject(parent) {
  m_purgeTimer = new QTimer(this);
  connect(m_purgeTimer, &QTimer::timeout, this,
          &EphemeralShotManager::handlePurgeTick);
  m_purgeTimer->start(10000); // Check every 10 seconds
}

int EphemeralShotManager::activeEphemeralCount() const {
  return static_cast<int>(m_records.size());
}

void EphemeralShotManager::registerEphemeralFile(const QString &filePath,
                                                 int ttlSeconds) {
  if (filePath.isEmpty() || !QFile::exists(filePath))
    return;

  EphemeralRecord rec;
  rec.filePath = filePath;
  rec.ttlSeconds = ttlSeconds;
  rec.expireAt = QDateTime::currentDateTime().addSecs(ttlSeconds);

  m_records.insert(filePath, rec);
  emit recordsChanged();
}

void EphemeralShotManager::cancelEphemeral(const QString &filePath) {
  if (m_records.remove(filePath) > 0) {
    emit recordsChanged();
  }
}

bool EphemeralShotManager::isEphemeral(const QString &filePath) const {
  return m_records.contains(filePath);
}

int EphemeralShotManager::remainingSeconds(const QString &filePath) const {
  if (!m_records.contains(filePath))
    return -1;
  qint64 diff =
      QDateTime::currentDateTime().secsTo(m_records[filePath].expireAt);
  return std::max(0, static_cast<int>(diff));
}

void EphemeralShotManager::handlePurgeTick() { checkAndPurgeExpired(); }

void EphemeralShotManager::checkAndPurgeExpired() {
  QDateTime now = QDateTime::currentDateTime();
  QStringList toDelete;

  for (auto it = m_records.begin(); it != m_records.end(); ++it) {
    if (it.value().expireAt <= now) {
      toDelete.append(it.key());
    }
  }

  for (const QString &path : toDelete) {
    // Secure overwrite before deletion
    QFile f(path);
    if (f.open(QIODevice::ReadWrite)) {
      qint64 size = f.size();
      QByteArray zeros(static_cast<int>(std::min<qint64>(size, 65536)), 0);
      f.write(zeros);
      f.flush();
      f.close();
    }
    QFile::remove(path);
    m_records.remove(path);
    emit filePurged(path);
  }

  if (!toDelete.isEmpty()) {
    emit recordsChanged();
  }
}

} // namespace ro_screenshot
