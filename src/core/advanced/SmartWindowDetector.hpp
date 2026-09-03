#pragma once

#include <QImage>
#include <QObject>
#include <QPoint>
#include <QRect>
#include <QVector>

namespace ro_screenshot {

class SmartWindowDetector : public QObject {
  Q_OBJECT

public:
  explicit SmartWindowDetector(QObject *parent = nullptr);
  ~SmartWindowDetector() override = default;

  // Snaps cursor or selection rectangle to nearest window / container boundary
  static QRect snapToWindowOrEdge(const QPoint &cursorPos,
                                  const QVector<QRect> &knownWindows,
                                  const QSize &screenSize,
                                  int snapThreshold = 18);

  // Heuristic edge & container detector based on desktop image
  static QVector<QRect> detectCandidateWindows(const QImage &desktopFrame,
                                               int minSize = 100);
};

} // namespace ro_screenshot
