#include "SmartDimensionGuide.hpp"
#include <QPainter>
#include <cmath>

namespace ro_screenshot {

SmartDimensionGuide::SmartDimensionGuide(QObject *parent) : QObject(parent) {}

DimensionMeasurement SmartDimensionGuide::measure(const QRect &a,
                                                  const QRect &b) {
  DimensionMeasurement m;
  m.boundsA = a;
  m.boundsB = b;
  m.targetWidth = a.width();
  m.targetHeight = a.height();

  if (a.x() + a.width() <= b.x()) {
    m.marginRight = b.x() - (a.x() + a.width());
  } else if (b.x() + b.width() <= a.x()) {
    m.marginLeft = a.x() - (b.x() + b.width());
  }

  if (a.y() + a.height() <= b.y()) {
    m.marginBottom = b.y() - (a.y() + a.height());
  } else if (b.y() + b.height() <= a.y()) {
    m.marginTop = a.y() - (b.y() + b.height());
  }

  m.distanceX = std::abs(a.center().x() - b.center().x());
  m.distanceY = std::abs(a.center().y() - b.center().y());
  return m;
}

DimensionMeasurement
SmartDimensionGuide::measureAgainstScreen(const QRect &element,
                                          const QSize &screenSize) {
  DimensionMeasurement m;
  m.boundsA = element;
  m.targetWidth = element.width();
  m.targetHeight = element.height();
  m.marginLeft = element.x();
  m.marginTop = element.y();
  m.marginRight = screenSize.width() - (element.x() + element.width());
  m.marginBottom = screenSize.height() - (element.y() + element.height());
  return m;
}

QRect SmartDimensionGuide::detectElementBoundsAt(const QImage &image,
                                                 const QPoint &pos,
                                                 int tolerance) {
  if (image.isNull() || !image.rect().contains(pos)) {
    return QRect(pos, QSize(100, 100));
  }

  QRgb seed = image.pixel(pos);
  int seedR = qRed(seed);
  int seedG = qGreen(seed);
  int seedB = qBlue(seed);

  auto colorDiff = [seedR, seedG, seedB](QRgb c) {
    return std::abs(qRed(c) - seedR) + std::abs(qGreen(c) - seedG) +
           std::abs(qBlue(c) - seedB);
  };

  int left = pos.x();
  while (left > 0 &&
         colorDiff(image.pixel(left - 1, pos.y())) < tolerance * 3) {
    left--;
  }

  int right = pos.x();
  while (right < image.width() - 1 &&
         colorDiff(image.pixel(right + 1, pos.y())) < tolerance * 3) {
    right++;
  }

  int top = pos.y();
  while (top > 0 && colorDiff(image.pixel(pos.x(), top - 1)) < tolerance * 3) {
    top--;
  }

  int bottom = pos.y();
  while (bottom < image.height() - 1 &&
         colorDiff(image.pixel(pos.x(), bottom + 1)) < tolerance * 3) {
    bottom++;
  }

  int w = right - left + 1;
  int h = bottom - top + 1;
  if (w < 4 || h < 4) {
    return QRect(pos.x() - 20, pos.y() - 20, 40, 40);
  }

  return QRect(left, top, w, h);
}

QImage SmartDimensionGuide::renderGuideOverlay(const QImage &baseImage,
                                               const QRect &element,
                                               const QSize &screenSize) {
  if (baseImage.isNull())
    return {};

  QImage overlay = baseImage.copy();
  QPainter painter(&overlay);
  painter.setRenderHint(QPainter::Antialiasing, true);

  DimensionMeasurement m = measureAgainstScreen(element, screenSize);

  QPen guidePen(QColor("#EF4444"), 1.5, Qt::DashLine);
  painter.setPen(guidePen);

  painter.drawLine(element.center().x(), 0, element.center().x(),
                   element.top());
  painter.drawLine(element.center().x(), element.bottom(), element.center().x(),
                   screenSize.height());
  painter.drawLine(0, element.center().y(), element.left(),
                   element.center().y());
  painter.drawLine(element.right(), element.center().y(), screenSize.width(),
                   element.center().y());

  painter.setPen(QPen(QColor("#EF4444"), 2.0));
  painter.setBrush(QColor(239, 68, 68, 30));
  painter.drawRect(element);

  auto drawBadge = [&painter](int x, int y, const QString &text) {
    QFont font = painter.font();
    font.setPixelSize(11);
    font.setBold(true);
    painter.setFont(font);

    QFontMetrics fm(font);
    int tw = fm.horizontalAdvance(text) + 12;
    int th = 20;

    QRect badgeRect(x - tw / 2, y - th / 2, tw, th);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#1E293B"));
    painter.drawRoundedRect(badgeRect, 4, 4);

    painter.setPen(Qt::white);
    painter.drawText(badgeRect, Qt::AlignCenter, text);
  };

  if (m.marginTop > 20)
    drawBadge(element.center().x(), m.marginTop / 2,
              QString::number(m.marginTop) + "px");
  if (m.marginBottom > 20)
    drawBadge(element.center().x(), element.bottom() + (m.marginBottom / 2),
              QString::number(m.marginBottom) + "px");
  if (m.marginLeft > 20)
    drawBadge(m.marginLeft / 2, element.center().y(),
              QString::number(m.marginLeft) + "px");
  if (m.marginRight > 20)
    drawBadge(element.right() + (m.marginRight / 2), element.center().y(),
              QString::number(m.marginRight) + "px");

  QString sizeText = QString("%1 × %2").arg(m.targetWidth).arg(m.targetHeight);
  drawBadge(element.center().x(), element.center().y(), sizeText);

  painter.end();
  return overlay;
}

} // namespace ro_screenshot
