#pragma once

#include <QColor>
#include <QImage>
#include <QObject>
#include <QPoint>
#include <QRect>
#include <QString>
#include <QVector>

namespace ro_screenshot {

struct DimensionMeasurement {
  QRect boundsA;
  QRect boundsB;
  int distanceX{0};
  int distanceY{0};
  int marginTop{0};
  int marginBottom{0};
  int marginLeft{0};
  int marginRight{0};
  int targetWidth{0};
  int targetHeight{0};
};

class SmartDimensionGuide : public QObject {
  Q_OBJECT

public:
  explicit SmartDimensionGuide(QObject *parent = nullptr);
  ~SmartDimensionGuide() override = default;

  static DimensionMeasurement measure(const QRect &elementA,
                                      const QRect &elementB);
  static DimensionMeasurement measureAgainstScreen(const QRect &element,
                                                   const QSize &screenSize);
  static QRect detectElementBoundsAt(const QImage &image, const QPoint &pos,
                                     int tolerance = 24);
  static QImage renderGuideOverlay(const QImage &baseImage,
                                   const QRect &element,
                                   const QSize &screenSize);
};

} // namespace ro_screenshot
