#pragma once

#include <QImage>
#include <QObject>
#include <QString>

namespace ro_screenshot {

class PerceptualHashEngine : public QObject {
  Q_OBJECT

public:
  explicit PerceptualHashEngine(QObject *parent = nullptr);
  ~PerceptualHashEngine() override = default;

  // Computes 64-bit Difference Hash (dHash) by downscaling to 9x8 grayscale
  static quint64 computeDHash(const QImage &image);

  // Computes 64-bit Average Hash (aHash) by downscaling to 8x8 grayscale
  static quint64 computeAHash(const QImage &image);

  // Calculates Hamming distance between two 64-bit hashes (0 to 64)
  static int hammingDistance(quint64 hash1, quint64 hash2);

  // Calculates similarity ratio (0.0 to 1.0)
  static double similarityScore(quint64 hash1, quint64 hash2);

  // Checks if two images are perceptual duplicates (threshold typically <= 10)
  static bool areDuplicates(quint64 hash1, quint64 hash2,
                            int maxHammingDistance = 8);

  static QString hashToHex(quint64 hash);
  static quint64 hexToHash(const QString &hex);
};

} // namespace ro_screenshot
