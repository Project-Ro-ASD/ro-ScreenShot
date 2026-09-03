#pragma once

#include <QClipboard>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include <QRegularExpression>
#include <QString>
#include <QUrl>
#include <QVariantMap>

namespace ro_screenshot {

struct TrashRecord {
  QString originalPath;
  QString trashedPath;
  QString fileName;
  QDateTime timestamp;
};

class TrashManager {
public:
  TrashManager() = default;

  bool isPathSafe(const QString &filePath, const QString &allowedRoot) const {
    if (filePath.isEmpty() || allowedRoot.isEmpty()) {
      return false;
    }
    QFileInfo fileInfo(filePath);
    QFileInfo rootInfo(allowedRoot);

    QString fileCanonical = fileInfo.canonicalFilePath();
    if (fileCanonical.isEmpty()) {
      fileCanonical = fileInfo.absoluteFilePath();
    }
    QString rootCanonical = rootInfo.canonicalFilePath();
    if (rootCanonical.isEmpty()) {
      rootCanonical = rootInfo.absoluteFilePath();
    }

    // Ensure filePath is inside rootCanonical
    if (!rootCanonical.endsWith('/')) {
      rootCanonical += '/';
    }
    return fileCanonical.startsWith(rootCanonical);
  }

  bool trashFile(const QString &filePath, const QString &allowedRoot,
                 QString *errorMsg = nullptr) {
    if (!isPathSafe(filePath, allowedRoot)) {
      if (errorMsg) {
        *errorMsg = "Güvenlik hatası: Belirtilen dosya ekran görüntüsü "
                    "kitaplığı dışında.";
      }
      return false;
    }

    QFileInfo info(filePath);
    if (!info.exists()) {
      if (errorMsg) {
        *errorMsg = "Dosya bulunamadı: " + filePath;
      }
      return false;
    }

    QString trashedPath;
    bool success = QFile::moveToTrash(filePath, &trashedPath);
    if (success) {
      QMutexLocker locker(&m_mutex);
      TrashRecord record;
      record.originalPath = info.absoluteFilePath();
      record.trashedPath = trashedPath;
      record.fileName = info.fileName();
      record.timestamp = QDateTime::currentDateTime();
      m_undoStack.append(record);
      // Keep undo stack bounded to last 50 items
      if (m_undoStack.size() > 50) {
        m_undoStack.removeFirst();
      }
      return true;
    } else {
      if (errorMsg) {
        *errorMsg = "Dosya çöp kutusuna taşınamadı.";
      }
      return false;
    }
  }

  bool canUndo() const {
    QMutexLocker locker(&m_mutex);
    return !m_undoStack.isEmpty();
  }

  TrashRecord lastTrashedRecord() const {
    QMutexLocker locker(&m_mutex);
    if (m_undoStack.isEmpty()) {
      return {};
    }
    return m_undoStack.last();
  }

  bool undoLastTrash(QString *restoredPath = nullptr,
                     QString *errorMsg = nullptr) {
    TrashRecord record;
    {
      QMutexLocker locker(&m_mutex);
      if (m_undoStack.isEmpty()) {
        if (errorMsg) {
          *errorMsg = "Geri alınacak silme işlemi bulunamadı.";
        }
        return false;
      }
      record = m_undoStack.takeLast();
    }

    // Attempt to restore
    if (!record.trashedPath.isEmpty() && QFile::exists(record.trashedPath)) {
      QFileInfo origInfo(record.originalPath);
      QDir().mkpath(origInfo.absolutePath());
      if (QFile::rename(record.trashedPath, record.originalPath)) {
        if (restoredPath) {
          *restoredPath = record.originalPath;
        }
        return true;
      }
    }

    if (errorMsg) {
      *errorMsg = "Dosya çöp kutusundan geri yüklenemedi.";
    }
    return false;
  }

  bool permanentDelete(const QString &filePath, const QString &allowedRoot,
                       QString *errorMsg = nullptr) {
    if (!isPathSafe(filePath, allowedRoot)) {
      if (errorMsg) {
        *errorMsg = "Güvenlik hatası: Belirtilen dosya ekran görüntüsü "
                    "kitaplığı dışında.";
      }
      return false;
    }

    QFile file(filePath);
    if (file.remove()) {
      return true;
    } else {
      if (errorMsg) {
        *errorMsg = "Dosya kalıcı olarak silinemedi.";
      }
      return false;
    }
  }

  QVariantMap renameFile(const QString &oldPath, const QString &newNameRaw,
                         const QString &allowedRoot) {
    QVariantMap result;
    result["success"] = false;

    if (!isPathSafe(oldPath, allowedRoot)) {
      result["error"] = "Güvenlik hatası: Dosya kitaplık sınırları dışında.";
      return result;
    }

    QString newName = newNameRaw.trimmed();
    if (newName.isEmpty()) {
      result["error"] = "Dosya adı boş olamaz.";
      return result;
    }

    // Disallow illegal characters
    static const QRegularExpression illegalChars(R"([\/\?\<\>\\:\*\|"])");
    if (newName.contains(illegalChars)) {
      result["error"] = "Dosya adı geçersiz karakterler içeremez.";
      return result;
    }

    QFileInfo oldInfo(oldPath);
    if (!oldInfo.exists()) {
      result["error"] = "Kaynak dosya bulunamadı.";
      return result;
    }

    // Preserve original extension if user didn't specify one
    QString suffix = oldInfo.suffix();
    if (!suffix.isEmpty() &&
        !newName.endsWith("." + suffix, Qt::CaseInsensitive)) {
      newName += "." + suffix;
    }

    QString newPath = oldInfo.dir().filePath(newName);
    if (QFile::exists(newPath)) {
      result["error"] = "Bu isimde bir dosya zaten mevcut.";
      return result;
    }

    if (QFile::rename(oldPath, newPath)) {
      result["success"] = true;
      result["newPath"] = newPath;
      result["newFileName"] = newName;
      return result;
    }

    result["error"] = "Yeniden adlandırma işlemi başarısız oldu.";
    return result;
  }

  void showInFolder(const QString &filePath) {
    QFileInfo info(filePath);
    if (!info.exists()) {
      return;
    }

    // Try org.freedesktop.FileManager1.ShowItems first
    bool dbusSuccess = false;
    if (QDBusConnection::sessionBus().isConnected()) {
      QDBusMessage msg = QDBusMessage::createMethodCall(
          "org.freedesktop.FileManager1", "/org/freedesktop/FileManager1",
          "org.freedesktop.FileManager1", "ShowItems");
      QStringList uriList;
      uriList << QUrl::fromLocalFile(info.absoluteFilePath()).toString();
      msg << uriList << QString("");
      QDBusMessage reply = QDBusConnection::sessionBus().call(msg);
      if (reply.type() != QDBusMessage::ErrorMessage) {
        dbusSuccess = true;
      }
    }

    // Fallback: open parent folder
    if (!dbusSuccess) {
      QDesktopServices::openUrl(QUrl::fromLocalFile(info.absolutePath()));
    }
  }

private:
  mutable QMutex m_mutex;
  QList<TrashRecord> m_undoStack;
};

} // namespace ro_screenshot
