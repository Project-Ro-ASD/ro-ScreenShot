#include "MockupFrameGenerator.hpp"
#include <QPainter>
#include <QPainterPath>
#include <cmath>

namespace ro_screenshot {

MockupFrameGenerator::MockupFrameGenerator(QObject *parent) : QObject(parent) {}

QLinearGradient
MockupFrameGenerator::createPresetGradient(MockupPreset preset,
                                           const QRect &targetRect) {
  QLinearGradient grad(targetRect.topLeft(), targetRect.bottomRight());
  switch (preset) {
  case MockupPreset::Sunset:
    grad.setColorAt(0.0, QColor("#FF512F"));
    grad.setColorAt(1.0, QColor("#DD2476"));
    break;
  case MockupPreset::Ocean:
    grad.setColorAt(0.0, QColor("#2193b0"));
    grad.setColorAt(1.0, QColor("#6dd5ed"));
    break;
  case MockupPreset::Cyberpunk:
    grad.setColorAt(0.0, QColor("#8A2387"));
    grad.setColorAt(0.5, QColor("#E94057"));
    grad.setColorAt(1.0, QColor("#F27121"));
    break;
  case MockupPreset::Velvet:
    grad.setColorAt(0.0, QColor("#1e130c"));
    grad.setColorAt(1.0, QColor("#9a8478"));
    break;
  case MockupPreset::Emerald:
    grad.setColorAt(0.0, QColor("#0ba360"));
    grad.setColorAt(1.0, QColor("#3cba92"));
    break;
  case MockupPreset::SlateDark:
    grad.setColorAt(0.0, QColor("#1e293b"));
    grad.setColorAt(1.0, QColor("#0f172a"));
    break;
  case MockupPreset::CustomSolid:
    grad.setColorAt(0.0, QColor("#3b82f6"));
    grad.setColorAt(1.0, QColor("#1d4ed8"));
    break;
  }
  return grad;
}

QString MockupFrameGenerator::presetName(MockupPreset preset) {
  switch (preset) {
  case MockupPreset::Sunset:
    return QStringLiteral("Sunset (Orange-Pink)");
  case MockupPreset::Ocean:
    return QStringLiteral("Ocean Breeze");
  case MockupPreset::Cyberpunk:
    return QStringLiteral("Cyberpunk Neon");
  case MockupPreset::Velvet:
    return QStringLiteral("Dark Velvet");
  case MockupPreset::Emerald:
    return QStringLiteral("Emerald Mint");
  case MockupPreset::SlateDark:
    return QStringLiteral("Slate Minimal");
  case MockupPreset::CustomSolid:
    return QStringLiteral("Custom Vibrant");
  }
  return QStringLiteral("Preset");
}

QImage MockupFrameGenerator::generate(const QImage &source,
                                      const MockupOptions &options) {
  if (source.isNull())
    return {};

  int headerHeight = options.drawWindowControls ? 32 : 0;
  int innerW = source.width();
  int innerH = source.height() + headerHeight;

  int totalW = innerW + (options.padding * 2);
  int totalH = innerH + (options.padding * 2);

  QImage output(totalW, totalH, QImage::Format_ARGB32_Premultiplied);
  output.fill(Qt::transparent);

  QPainter painter(&output);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

  // 1. Draw Background Gradient
  QRect canvasRect(0, 0, totalW, totalH);
  QLinearGradient bgGrad = createPresetGradient(options.preset, canvasRect);
  if (options.preset == MockupPreset::CustomSolid) {
    bgGrad = QLinearGradient(canvasRect.topLeft(), canvasRect.bottomRight());
    bgGrad.setColorAt(0.0, options.customColor1);
    bgGrad.setColorAt(1.0, options.customColor2);
  }
  painter.fillRect(canvasRect, bgGrad);

  // 2. Draw Soft Drop Shadow
  QRectF innerRect(options.padding, options.padding, innerW, innerH);
  if (options.shadowBlur > 0) {
    int passes = 5;
    for (int i = passes; i >= 1; --i) {
      qreal spread = (static_cast<qreal>(options.shadowBlur) / passes) * i;
      qreal alpha =
          static_cast<qreal>(options.shadowColor.alpha()) / (passes * 1.5);
      QColor c = options.shadowColor;
      c.setAlpha(std::clamp(static_cast<int>(alpha), 1, 255));
      painter.setPen(Qt::NoPen);
      painter.setBrush(c);
      QRectF shadowRect = innerRect.adjusted(
          -spread * 0.4,
          options.shadowOffsetY * (spread / options.shadowBlur) - spread * 0.2,
          spread * 0.4,
          options.shadowOffsetY * (spread / options.shadowBlur) + spread * 0.4);
      painter.drawRoundedRect(shadowRect, options.cornerRadius + spread * 0.3,
                              options.cornerRadius + spread * 0.3);
    }
  }

  // 3. Draw Window Container Card
  QPainterPath cardPath;
  cardPath.addRoundedRect(innerRect, options.cornerRadius,
                          options.cornerRadius);
  painter.save();
  painter.setClipPath(cardPath);

  // Top header if requested
  if (options.drawWindowControls) {
    QRectF headerRect(innerRect.x(), innerRect.y(), innerRect.width(),
                      headerHeight);
    painter.fillRect(headerRect, QColor(30, 30, 36, 240));

    // Traffic light dots
    qreal dotY = innerRect.y() + (headerHeight / 2.0);
    qreal dotRadius = 5.0;
    qreal startX = innerRect.x() + 16.0;

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#FF5F56")); // Red
    painter.drawEllipse(QPointF(startX, dotY), dotRadius, dotRadius);

    painter.setBrush(QColor("#FFBD2E")); // Yellow
    painter.drawEllipse(QPointF(startX + 16.0, dotY), dotRadius, dotRadius);

    painter.setBrush(QColor("#27C93F")); // Green
    painter.drawEllipse(QPointF(startX + 32.0, dotY), dotRadius, dotRadius);
  }

  // Draw image content
  QRectF imgTarget(innerRect.x(), innerRect.y() + headerHeight, source.width(),
                   source.height());
  painter.drawImage(imgTarget, source);
  painter.restore();

  // Subtle border highlight
  painter.setPen(QPen(QColor(255, 255, 255, 40), 1.0));
  painter.setBrush(Qt::NoBrush);
  painter.drawRoundedRect(innerRect, options.cornerRadius,
                          options.cornerRadius);

  painter.end();
  return output;
}

} // namespace ro_screenshot
