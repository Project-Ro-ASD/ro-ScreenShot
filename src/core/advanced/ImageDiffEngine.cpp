#include "ImageDiffEngine.hpp"
#include <QPainter>
#include <algorithm>
#include <cmath>

namespace ro_screenshot {

ImageDiffEngine::ImageDiffEngine(QObject *parent) : QObject(parent) {}

DiffResult ImageDiffEngine::compare(const QImage &imgA, const QImage &imgB,
                                    int colorThreshold,
                                    const QColor &diffColor) {
  DiffResult result;
  if (imgA.isNull() || imgB.isNull()) {
    result.dimensionsMatch = false;
    return result;
  }

  int w = std::max(imgA.width(), imgB.width());
  int h = std::max(imgA.height(), imgB.height());

  result.dimensionsMatch =
      (imgA.width() == imgB.width() && imgA.height() == imgB.height());
  result.totalPixels = w * h;

  QImage a = imgA.convertToFormat(QImage::Format_ARGB32);
  QImage b = imgB.convertToFormat(QImage::Format_ARGB32);

  result.diffImage = QImage(w, h, QImage::Format_ARGB32);
  result.diffImage.fill(Qt::transparent);

  int diffCount = 0;

  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      if (x >= a.width() || y >= a.height() || x >= b.width() ||
          y >= b.height()) {
        result.diffImage.setPixelColor(x, y, diffColor);
        diffCount++;
        continue;
      }

      QRgb pxA = a.pixel(x, y);
      QRgb pxB = b.pixel(x, y);

      int dr = std::abs(qRed(pxA) - qRed(pxB));
      int dg = std::abs(qGreen(pxA) - qGreen(pxB));
      int db = std::abs(qBlue(pxA) - qBlue(pxB));

      if (dr > colorThreshold || dg > colorThreshold || db > colorThreshold) {
        // Highlight in diff color with red tint
        result.diffImage.setPixelColor(x, y, diffColor);
        diffCount++;
      } else {
        // Dim unchanged pixel
        QColor dim = QColor::fromRgb(pxA);
        dim.setAlpha(60);
        result.diffImage.setPixelColor(x, y, dim);
      }
    }
  }

  result.differentPixels = diffCount;
  result.differencePercent =
      (static_cast<double>(diffCount) / result.totalPixels) * 100.0;
  result.similarityPercent = 100.0 - result.differencePercent;

  // Render side-by-side
  result.sideBySideImage = QImage(w * 2 + 10, h, QImage::Format_ARGB32);
  result.sideBySideImage.fill(QColor("#0F172A"));
  QPainter painter(&result.sideBySideImage);
  painter.drawImage(0, 0, imgA);
  painter.drawImage(w + 10, 0, imgB);
  painter.end();

  return result;
}

QImage ImageDiffEngine::renderSplitView(const QImage &imgA, const QImage &imgB,
                                        double splitRatio,
                                        bool showDividerLine) {
  if (imgA.isNull())
    return imgB;
  if (imgB.isNull())
    return imgA;

  int w = std::max(imgA.width(), imgB.width());
  int h = std::max(imgA.height(), imgB.height());
  splitRatio = std::clamp(splitRatio, 0.0, 1.0);

  int splitX = static_cast<int>(w * splitRatio);

  QImage output(w, h, QImage::Format_ARGB32);
  QPainter painter(&output);

  // Draw full Image B in background
  painter.drawImage(0, 0, imgB);

  // Clip and draw Image A on the left side
  painter.save();
  painter.setClipRect(0, 0, splitX, h);
  painter.drawImage(0, 0, imgA);
  painter.restore();

  // Draw vertical slider divider line
  if (showDividerLine) {
    painter.setPen(QPen(QColor("#38BDF8"), 2));
    painter.drawLine(splitX, 0, splitX, h);

    // Draw circular grab handle in the middle
    int handleY = h / 2;
    painter.setBrush(QColor("#38BDF8"));
    painter.setPen(QPen(Qt::white, 2));
    painter.drawEllipse(QPoint(splitX, handleY), 12, 12);
  }

  painter.end();
  return output;
}

} // namespace ro_screenshot
