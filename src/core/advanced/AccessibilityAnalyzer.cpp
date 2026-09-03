#include "AccessibilityAnalyzer.hpp"
#include <algorithm>
#include <cmath>

namespace ro_screenshot {

AccessibilityAnalyzer::AccessibilityAnalyzer(QObject *parent)
    : QObject(parent) {}

double AccessibilityAnalyzer::relativeLuminance(const QColor &c) {
  auto channelLum = [](double val) {
    val = val / 255.0;
    if (val <= 0.04045) {
      return val / 12.92;
    }
    return std::pow((val + 0.055) / 1.055, 2.4);
  };

  double r = channelLum(c.redF() * 255.0);
  double g = channelLum(c.greenF() * 255.0);
  double b = channelLum(c.blueF() * 255.0);

  return 0.2126 * r + 0.7152 * g + 0.0722 * b;
}

WcagContrastResult AccessibilityAnalyzer::evaluateContrast(const QColor &fg,
                                                           const QColor &bg) {
  WcagContrastResult res;
  res.foreground = fg;
  res.background = bg;

  double l1 = relativeLuminance(fg);
  double l2 = relativeLuminance(bg);

  if (l1 < l2)
    std::swap(l1, l2);

  res.ratio = (l1 + 0.05) / (l2 + 0.05);
  res.passesAaNormal = (res.ratio >= 4.5);
  res.passesAaLarge = (res.ratio >= 3.0);
  res.passesAaaNormal = (res.ratio >= 7.0);
  res.passesAaaLarge = (res.ratio >= 4.5);

  if (res.passesAaaNormal) {
    res.grade = QStringLiteral("AAA");
  } else if (res.passesAaNormal) {
    res.grade = QStringLiteral("AA");
  } else if (res.passesAaLarge) {
    res.grade = QStringLiteral("AA (Large Text Only)");
  } else {
    res.grade = QStringLiteral("Fail");
  }

  return res;
}

QColor AccessibilityAnalyzer::simulateColor(const QColor &c,
                                            ColorBlindnessType type) {
  if (type == ColorBlindnessType::None)
    return c;

  double r = c.redF();
  double g = c.greenF();
  double b = c.blueF();
  double nr = r, ng = g, nb = b;

  switch (type) {
  case ColorBlindnessType::Protanopia: // Red-blind
    nr = 0.56667 * r + 0.43333 * g + 0.0 * b;
    ng = 0.55833 * r + 0.44167 * g + 0.0 * b;
    nb = 0.0 * r + 0.24167 * g + 0.75833 * b;
    break;
  case ColorBlindnessType::Deuteranopia: // Green-blind
    nr = 0.625 * r + 0.375 * g + 0.0 * b;
    ng = 0.70 * r + 0.30 * g + 0.0 * b;
    nb = 0.0 * r + 0.30 * g + 0.70 * b;
    break;
  case ColorBlindnessType::Tritanopia: // Blue-blind
    nr = 0.95 * r + 0.05 * g + 0.0 * b;
    ng = 0.0 * r + 0.43333 * g + 0.56667 * b;
    nb = 0.0 * r + 0.475 * g + 0.525 * b;
    break;
  case ColorBlindnessType::Achromatopsia: // Monochromacy
    nr = 0.299 * r + 0.587 * g + 0.114 * b;
    ng = nr;
    nb = nr;
    break;
  default:
    break;
  }

  nr = std::clamp(nr, 0.0, 1.0);
  ng = std::clamp(ng, 0.0, 1.0);
  nb = std::clamp(nb, 0.0, 1.0);

  return QColor::fromRgbF(static_cast<float>(nr), static_cast<float>(ng),
                          static_cast<float>(nb),
                          static_cast<float>(c.alphaF()));
}

QImage AccessibilityAnalyzer::simulateColorBlindness(const QImage &source,
                                                     ColorBlindnessType type) {
  if (source.isNull() || type == ColorBlindnessType::None)
    return source;

  QImage sim = source.convertToFormat(QImage::Format_ARGB32);
  for (int y = 0; y < sim.height(); ++y) {
    QRgb *line = reinterpret_cast<QRgb *>(sim.scanLine(y));
    for (int x = 0; x < sim.width(); ++x) {
      QColor orig(line[x]);
      QColor converted = simulateColor(orig, type);
      line[x] = converted.rgba();
    }
  }
  return sim;
}

} // namespace ro_screenshot
