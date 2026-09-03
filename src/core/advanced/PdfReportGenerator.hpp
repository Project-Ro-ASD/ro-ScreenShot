#pragma once

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

namespace ro_screenshot {

struct PdfReportEntry {
  QString imagePath;
  QString title;
  QString description;
  QString tags;
  QDateTime timestamp;
};

struct PdfReportOptions {
  QString title{QStringLiteral("ro-ScreenShot Bug & UI Report")};
  QString author{QStringLiteral("ro-ScreenShot User")};
  QString organization{QStringLiteral("Project Ro-ASD")};
  QString notes;
  bool includeSystemInfo{true};
  bool includeTimestamps{true};
  bool includeTableOfContents{true};
};

class PdfReportGenerator : public QObject {
  Q_OBJECT

public:
  explicit PdfReportGenerator(QObject *parent = nullptr);
  ~PdfReportGenerator() override = default;

  static bool generateReport(const QString &outputPath,
                             const QVector<PdfReportEntry> &entries,
                             const PdfReportOptions &options);
};

} // namespace ro_screenshot
