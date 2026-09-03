#pragma once

#include <QColor>
#include <QImage>
#include <QObject>
#include <QString>

namespace ro_screenshot {

struct DiffResult {
  QImage diffImage;
  QImage sideBySideImage;
  int differentPixels{0};
  int totalPixels{0};
  double differencePercent{0.0};
  double similarityPercent{100.0};
  bool dimensionsMatch{true};
};

class ImageDiffEngine : public QObject {
  Q_OBJECT

public:
  explicit ImageDiffEngine(QObject *parent = nullptr);
  ~ImageDiffEngine() override = default;

  // Computes pixel difference with configurable color threshold and diff tint
  // color
  static DiffResult compare(const QImage &imgA, const QImage &imgB,
                            int colorThreshold = 16,
                            const QColor &diffColor = QColor("#EF4444"));

  // Creates split slider composition (0.0 to 1.0)
  static QImage renderSplitView(const QImage &imgA, const QImage &imgB,
                                double splitRatio = 0.5,
                                bool showDividerLine = true);
};

} // namespace ro_screenshot
