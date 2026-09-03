#include "SmartWindowDetector.hpp"
#include <algorithm>
#include <cmath>

namespace ro_screenshot {

SmartWindowDetector::SmartWindowDetector(QObject *parent) : QObject(parent) {}

QRect SmartWindowDetector::snapToWindowOrEdge(
    const QPoint &cursorPos, const QVector<QRect> &knownWindows,
    const QSize &screenSize, int snapThreshold) {
  for (const QRect &win : knownWindows) {
    if (win.contains(cursorPos)) {
      return win;
    }
    if (std::abs(cursorPos.x() - win.left()) <= snapThreshold ||
        std::abs(cursorPos.x() - win.right()) <= snapThreshold ||
        std::abs(cursorPos.y() - win.top()) <= snapThreshold ||
        std::abs(cursorPos.y() - win.bottom()) <= snapThreshold) {
      return win;
    }
  }

  int halfW = screenSize.width() / 2;
  int halfH = screenSize.height() / 2;

  if (cursorPos.x() < halfW && cursorPos.y() < halfH) {
    return QRect(0, 0, halfW, halfH);
  } else if (cursorPos.x() >= halfW && cursorPos.y() < halfH) {
    return QRect(halfW, 0, halfW, halfH);
  } else if (cursorPos.x() < halfW && cursorPos.y() >= halfH) {
    return QRect(0, halfH, halfW, halfH);
  } else {
    return QRect(halfW, halfH, halfW, halfH);
  }
}

QVector<QRect>
SmartWindowDetector::detectCandidateWindows(const QImage &desktopFrame,
                                            int /*minSize*/) {
  QVector<QRect> candidates;
  if (desktopFrame.isNull())
    return candidates;

  int w = desktopFrame.width();
  int h = desktopFrame.height();

  candidates.append(QRect(0, 0, w, h));
  candidates.append(QRect(0, 0, w / 2, h));
  candidates.append(QRect(w / 2, 0, w / 2, h));
  candidates.append(QRect(w / 4, h / 4, w / 2, h / 2));
  candidates.append(QRect(w / 6, h / 6, (w * 2) / 3, (h * 2) / 3));

  return candidates;
}

} // namespace ro_screenshot
