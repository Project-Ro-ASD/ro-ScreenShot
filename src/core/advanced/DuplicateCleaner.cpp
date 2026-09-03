#include "DuplicateCleaner.hpp"
#include <QFile>
#include <QFileInfo>
#include <QImage>

namespace ro_screenshot {

DuplicateCleaner::DuplicateCleaner(QObject *parent) : QObject(parent) {}

QVector<DuplicateGroup>
DuplicateCleaner::findDuplicates(const QStringList &filePaths, int tolerance) {
  struct ImageHash {
    QString path;
    quint64 hash;
    qint64 size;
  };
  QVector<ImageHash> hashes;
  hashes.reserve(filePaths.size());

  for (const QString &p : filePaths) {
    QFileInfo fi(p);
    if (!fi.exists())
      continue;
    QImage img(p);
    if (img.isNull())
      continue;
    quint64 h = PerceptualHashEngine::computeDHash(img);
    hashes.append({p, h, fi.size()});
  }

  QVector<DuplicateGroup> groups;
  QSet<QString> processed;

  for (int i = 0; i < hashes.size(); ++i) {
    if (processed.contains(hashes[i].path))
      continue;

    DuplicateGroup grp;
    grp.primaryFilePath = hashes[i].path;
    grp.hash = hashes[i].hash;

    for (int j = i + 1; j < hashes.size(); ++j) {
      if (processed.contains(hashes[j].path))
        continue;

      if (PerceptualHashEngine::areDuplicates(hashes[i].hash, hashes[j].hash,
                                              tolerance)) {
        grp.duplicateFilePaths.append(hashes[j].path);
        grp.totalReclaimableBytes += hashes[j].size;
        processed.insert(hashes[j].path);
      }
    }

    if (!grp.duplicateFilePaths.isEmpty()) {
      processed.insert(hashes[i].path);
      groups.append(grp);
    }
  }

  return groups;
}

qint64 DuplicateCleaner::cleanDuplicates(const QVector<DuplicateGroup> &groups,
                                         bool moveToTrash) {
  qint64 freed = 0;
  for (const auto &g : groups) {
    for (const QString &dup : g.duplicateFilePaths) {
      QFileInfo fi(dup);
      qint64 sz = fi.size();
      bool ok = false;
      if (moveToTrash) {
        ok = QFile::moveToTrash(dup);
      } else {
        ok = QFile::remove(dup);
      }
      if (ok) {
        freed += sz;
      }
    }
  }
  return freed;
}

} // namespace ro_screenshot
