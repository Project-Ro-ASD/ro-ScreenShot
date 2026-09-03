#pragma once

#include <QColor>
#include <QImage>
#include <QObject>
#include <QString>

namespace ro_screenshot {

enum class ColorBlindnessType {
  None,
  Protanopia,   // Red-blind
  Deuteranopia, // Green-blind
  Tritanopia,   // Blue-blind
  Achromatopsia // Monochromacy
};

struct WcagContrastResult {
  double ratio{1.0};
  bool passesAaNormal{false};  // 4.5:1
  bool passesAaLarge{false};   // 3.0:1
  bool passesAaaNormal{false}; // 7.0:1
  bool passesAaaLarge{false};  // 4.5:1
  QString grade;               // "AAA", "AA", "Fail"
  QColor foreground;
  QColor background;
};

class AccessibilityAnalyzer : public QObject {
  Q_OBJECT

public:
  explicit AccessibilityAnalyzer(QObject *parent = nullptr);
  ~AccessibilityAnalyzer() override = default;

  // WCAG 2.1 Relative Luminance
  static double relativeLuminance(const QColor &c);

  // WCAG 2.1 Contrast Ratio: (L1 + 0.05) / (L2 + 0.05)
  static WcagContrastResult evaluateContrast(const QColor &fg,
                                             const QColor &bg);

  // Color Blindness Simulation
  static QImage simulateColorBlindness(const QImage &source,
                                       ColorBlindnessType type);
  static QColor simulateColor(const QColor &c, ColorBlindnessType type);
};

} // namespace ro_screenshot
