#include "TableExtractor.hpp"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <algorithm>

namespace ro_screenshot {

TableExtractor::TableExtractor(QObject *parent) : QObject(parent) {}

ExtractedTable
TableExtractor::extractFromOcr(const QVector<OcrTextBlock> &blocks,
                               int yThreshold) {
  ExtractedTable table;
  if (blocks.isEmpty())
    return table;

  // Sort blocks top-to-bottom, then left-to-right
  QVector<OcrTextBlock> sortedBlocks = blocks;
  std::sort(sortedBlocks.begin(), sortedBlocks.end(),
            [](const OcrTextBlock &a, const OcrTextBlock &b) {
              if (std::abs(a.boundingBox.y() - b.boundingBox.y()) > 10) {
                return a.boundingBox.y() < b.boundingBox.y();
              }
              return a.boundingBox.x() < b.boundingBox.x();
            });

  // Group into rows based on Y proximity
  struct RowGroup {
    int avgY{0};
    QVector<OcrTextBlock> cells;
  };
  QVector<RowGroup> rowGroups;

  for (const auto &b : sortedBlocks) {
    bool foundRow = false;
    for (auto &rg : rowGroups) {
      if (std::abs(b.boundingBox.center().y() - rg.avgY) <= yThreshold) {
        rg.cells.append(b);
        rg.avgY = (rg.avgY + b.boundingBox.center().y()) / 2;
        foundRow = true;
        break;
      }
    }
    if (!foundRow) {
      RowGroup newRg;
      newRg.avgY = b.boundingBox.center().y();
      newRg.cells.append(b);
      rowGroups.append(newRg);
    }
  }

  int maxCols = 0;
  for (auto &rg : rowGroups) {
    std::sort(rg.cells.begin(), rg.cells.end(),
              [](const OcrTextBlock &a, const OcrTextBlock &b) {
                return a.boundingBox.x() < b.boundingBox.x();
              });
    QStringList rowData;
    for (const auto &c : rg.cells) {
      rowData.append(c.text.trimmed());
    }
    maxCols = std::max(maxCols, static_cast<int>(rowData.size()));
    table.rows.append(rowData);
  }

  table.columnCount = maxCols;
  table.rowCount = static_cast<int>(table.rows.size());
  table.isValid = (table.rowCount > 0 && table.columnCount > 0);
  return table;
}

ExtractedTable TableExtractor::extractFromText(const QString &rawText) {
  ExtractedTable table;
  const QStringList lines = rawText.split('\n', Qt::SkipEmptyParts);
  int maxCols = 0;

  for (const QString &line : lines) {
    QString trimmed = line.trimmed();
    QStringList cols;
    if (trimmed.contains('\t')) {
      cols = trimmed.split('\t');
    } else if (trimmed.contains('|')) {
      cols = trimmed.split('|', Qt::SkipEmptyParts);
    } else if (trimmed.contains(',')) {
      cols = trimmed.split(',');
    } else {
      cols = trimmed.split(QRegularExpression(QStringLiteral(R"(\s{2,})")));
    }

    QStringList cleanCols;
    for (const auto &c : cols) {
      cleanCols.append(c.trimmed());
    }
    if (!cleanCols.isEmpty()) {
      maxCols = std::max(maxCols, static_cast<int>(cleanCols.size()));
      table.rows.append(cleanCols);
    }
  }

  table.columnCount = maxCols;
  table.rowCount = static_cast<int>(table.rows.size());
  table.isValid = (table.rowCount > 0 && table.columnCount > 0);
  return table;
}

QString TableExtractor::toMarkdown(const ExtractedTable &table) {
  if (!table.isValid || table.rows.isEmpty())
    return {};

  QString md;
  // Header row
  const QStringList &header = table.rows.first();
  md += "|";
  for (int i = 0; i < table.columnCount; ++i) {
    md +=
        " " +
        (i < header.size() ? header[i] : QStringLiteral("Col %1").arg(i + 1)) +
        " |";
  }
  md += "\n|";
  for (int i = 0; i < table.columnCount; ++i) {
    md += " --- |";
  }
  md += "\n";

  // Data rows
  for (int r = 1; r < table.rows.size(); ++r) {
    const QStringList &row = table.rows[r];
    md += "|";
    for (int c = 0; c < table.columnCount; ++c) {
      md += " " + (c < row.size() ? row[c] : QString()) + " |";
    }
    md += "\n";
  }
  return md;
}

QString TableExtractor::toCsv(const ExtractedTable &table) {
  if (!table.isValid)
    return {};
  QString csv;
  for (const auto &row : table.rows) {
    QStringList escapedRow;
    for (const auto &cell : row) {
      if (cell.contains(',') || cell.contains('"') || cell.contains('\n')) {
        QString escaped = cell;
        escaped.replace('"', "\"\"");
        escapedRow.append(QString("\"%1\"").arg(escaped));
      } else {
        escapedRow.append(cell);
      }
    }
    csv += escapedRow.join(',') + "\n";
  }
  return csv;
}

QString TableExtractor::toJson(const ExtractedTable &table) {
  if (!table.isValid || table.rows.isEmpty())
    return "[]";

  QJsonArray jsonArray;
  const QStringList &header = table.rows.first();

  for (int r = 1; r < table.rows.size(); ++r) {
    const QStringList &row = table.rows[r];
    QJsonObject rowObj;
    for (int c = 0; c < table.columnCount; ++c) {
      QString key = (c < header.size() && !header[c].isEmpty())
                        ? header[c]
                        : QString("column_%1").arg(c + 1);
      QString val = (c < row.size()) ? row[c] : QString();
      rowObj[key] = val;
    }
    jsonArray.append(rowObj);
  }
  return QString::fromUtf8(
      QJsonDocument(jsonArray).toJson(QJsonDocument::Indented));
}

} // namespace ro_screenshot
