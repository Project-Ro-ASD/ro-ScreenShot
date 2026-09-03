#pragma once

#include "core/advanced/OcrEngine.hpp"
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QVector>

namespace ro_screenshot {

struct ExtractedTable {
  QVector<QStringList> rows;
  int columnCount{0};
  int rowCount{0};
  bool isValid{false};
};

class TableExtractor : public QObject {
  Q_OBJECT

public:
  explicit TableExtractor(QObject *parent = nullptr);
  ~TableExtractor() override = default;

  static ExtractedTable extractFromOcr(const QVector<OcrTextBlock> &blocks,
                                       int yThreshold = 12);
  static ExtractedTable extractFromText(const QString &rawText);
  static QString toMarkdown(const ExtractedTable &table);
  static QString toCsv(const ExtractedTable &table);
  static QString toJson(const ExtractedTable &table);
};

} // namespace ro_screenshot
