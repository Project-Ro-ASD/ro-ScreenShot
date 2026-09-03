#pragma once

#include <QColor>
#include <QImage>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QVector>

namespace ro_screenshot {

struct PaletteColor {
  QColor color;
  QString hex;
  QString rgb;
  QString hsl;
  QString tailwindClass;
  QString tailwindName;
  double dominancePercent{0.0};
  int pixelCount{0};
};

class PaletteExtractor : public QObject {
  Q_OBJECT

public:
  explicit PaletteExtractor(QObject *parent = nullptr);
  ~PaletteExtractor() override = default;

  static QVector<PaletteColor> extract(const QImage &image, int maxColors = 6);
  static QString matchTailwindColor(const QColor &color);
  static QString exportAsTailwindConfig(const QVector<PaletteColor> &palette);
  static QString exportAsCssVariables(const QVector<PaletteColor> &palette);
  static QString exportAsJson(const QVector<PaletteColor> &palette);
};

} // namespace ro_screenshot
