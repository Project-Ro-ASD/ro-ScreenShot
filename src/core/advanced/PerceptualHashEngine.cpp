#include "PerceptualHashEngine.hpp"
#include <cmath>

namespace ro_screenshot {

PerceptualHashEngine::PerceptualHashEngine(QObject *parent) : QObject(parent) {}

quint64 PerceptualHashEngine::computeDHash(const QImage &image) {
  if (image.isNull())
    return 0;

  // 1. Scale to 9x8 grayscale
  QImage scaled =
      image.scaled(9, 8, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  if (scaled.format() != QImage::Format_Grayscale8) {
    scaled = scaled.convertToFormat(QImage::Format_Grayscale8);
  }

  quint64 hash = 0;
  // 2. Compare adjacent pixels
  for (int y = 0; y < 8; ++y) {
    const uchar *line = scaled.constScanLine(y);
    for (int x = 0; x < 8; ++x) {
      if (line[x] > line[x + 1]) {
        hash |= (1ULL << (y * 8 + x));
      }
    }
  }
  return hash;
}

quint64 PerceptualHashEngine::computeAHash(const QImage &image) {
  if (image.isNull())
    return 0;

  // 1. Scale to 8x8 grayscale
  QImage scaled =
      image.scaled(8, 8, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  if (scaled.format() != QImage::Format_Grayscale8) {
    scaled = scaled.convertToFormat(QImage::Format_Grayscale8);
  }

  // 2. Compute average
  quint32 sum = 0;
  for (int y = 0; y < 8; ++y) {
    const uchar *line = scaled.constScanLine(y);
    for (int x = 0; x < 8; ++x) {
      sum += line[x];
    }
  }
  uchar avg = static_cast<uchar>(sum / 64);

  // 3. Compute bits above average
  quint64 hash = 0;
  for (int y = 0; y < 8; ++y) {
    const uchar *line = scaled.constScanLine(y);
    for (int x = 0; x < 8; ++x) {
      if (line[x] >= avg) {
        hash |= (1ULL << (y * 8 + x));
      }
    }
  }
  return hash;
}

int PerceptualHashEngine::hammingDistance(quint64 hash1, quint64 hash2) {
  quint64 x = hash1 ^ hash2;
#if defined(__GNUC__) || defined(__clang__)
  return __builtin_popcountll(x);
#else
  int count = 0;
  while (x > 0) {
    count += (x & 1);
    x >>= 1;
  }
  return count;
#endif
}

double PerceptualHashEngine::similarityScore(quint64 hash1, quint64 hash2) {
  int dist = hammingDistance(hash1, hash2);
  return 1.0 - (static_cast<double>(dist) / 64.0);
}

bool PerceptualHashEngine::areDuplicates(quint64 hash1, quint64 hash2,
                                         int maxHammingDistance) {
  return hammingDistance(hash1, hash2) <= maxHammingDistance;
}

QString PerceptualHashEngine::hashToHex(quint64 hash) {
  return QString("%1").arg(hash, 16, 16, QChar('0')).toUpper();
}

quint64 PerceptualHashEngine::hexToHash(const QString &hex) {
  bool ok = false;
  return hex.toULongLong(&ok, 16);
}

} // namespace ro_screenshot
