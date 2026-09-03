#pragma once

#include "PerceptualHashEngine.hpp"
#include <QObject>
#include <QStringList>
#include <QVector>

namespace ro_screenshot {

struct DuplicateGroup {
  QString primaryFilePath;
  QStringList duplicateFilePaths;
  quint64 hash{0};
  qint64 totalReclaimableBytes{0};
};

class DuplicateCleaner : public QObject {
  Q_OBJECT

public:
  explicit DuplicateCleaner(QObject *parent = nullptr);
  ~DuplicateCleaner() override = default;

  static QVector<DuplicateGroup> findDuplicates(const QStringList &filePaths,
                                                int tolerance = 6);
  static qint64 cleanDuplicates(const QVector<DuplicateGroup> &groups,
                                bool moveToTrash = true);
};

} // namespace ro_screenshot
