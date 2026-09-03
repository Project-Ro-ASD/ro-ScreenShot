#include "ScrollingCaptureEngine.hpp"
#include <QPainter>
#include <algorithm>
#include <cmath>

namespace ro_screenshot {

ScrollingCaptureEngine::ScrollingCaptureEngine(QObject *parent)
    : QObject(parent) {}

int ScrollingCaptureEngine::findVerticalOverlap(const QImage &topFrame,
                                                const QImage &bottomFrame,
                                                int maxSearch) {
  if (topFrame.isNull() || bottomFrame.isNull())
    return 0;
  if (topFrame.width() != bottomFrame.width())
    return 0;

  int w = topFrame.width();
  int hTop = topFrame.height();
  int hBottom = bottomFrame.height();

  int searchLimit = std::min({maxSearch, hTop, hBottom});
  if (searchLimit <= 10)
    return 0;

  QImage f1 = topFrame.convertToFormat(QImage::Format_Grayscale8);
  QImage f2 = bottomFrame.convertToFormat(QImage::Format_Grayscale8);

  int bestOverlap = 0;
  double minDiff = 1e9;

  // Test different vertical overlap heights (e.g. from 20px to searchLimit)
  for (int overlap = 20; overlap <= searchLimit; overlap += 2) {
    double totalDiff = 0.0;
    int samples = 0;

    int sampleStep = std::max(1, w / 40); // 40 horizontal samples

    for (int y = 0; y < overlap; y += 2) {
      const uchar *line1 = f1.constScanLine(hTop - overlap + y);
      const uchar *line2 = f2.constScanLine(y);

      for (int x = 0; x < w; x += sampleStep) {
        totalDiff +=
            std::abs(static_cast<int>(line1[x]) - static_cast<int>(line2[x]));
        samples++;
      }
    }

    if (samples > 0) {
      double avgDiff = totalDiff / samples;
      if (avgDiff < minDiff) {
        minDiff = avgDiff;
        bestOverlap = overlap;
      }
    }
  }

  // If match error is sufficiently low (< 12 gray levels)
  if (minDiff < 15.0) {
    return bestOverlap;
  }
  return 0;
}

QImage ScrollingCaptureEngine::stitchFrames(const QVector<QImage> &frames,
                                            int maxOverlapSearch) {
  if (frames.isEmpty())
    return {};
  if (frames.size() == 1)
    return frames.first();

  QImage current = frames[0];

  for (int i = 1; i < frames.size(); ++i) {
    const QImage &next = frames[i];
    int overlap = findVerticalOverlap(current, next, maxOverlapSearch);

    int appendHeight = next.height() - overlap;
    if (appendHeight <= 0)
      continue;

    int newW = std::max(current.width(), next.width());
    int newH = current.height() + appendHeight;

    QImage stitched(newW, newH, QImage::Format_ARGB32);
    QPainter p(&stitched);
    p.drawImage(0, 0, current);
    p.drawImage(0, current.height(),
                next.copy(0, overlap, next.width(), appendHeight));
    p.end();

    current = stitched;
  }

  return current;
}

} // namespace ro_screenshot
