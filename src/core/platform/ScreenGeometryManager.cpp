#include "ScreenGeometryManager.hpp"
#include <QCursor>
#include <QGuiApplication>
#include <algorithm>

namespace ro_screenshot {

ScreenGeometryManager::ScreenGeometryManager(QObject *parent)
    : QObject(parent) {
  if (qGuiApp) {
    connect(qGuiApp, &QGuiApplication::screenAdded, this,
            &ScreenGeometryManager::handleScreenAdded);
    connect(qGuiApp, &QGuiApplication::screenRemoved, this,
            &ScreenGeometryManager::handleScreenRemoved);
    connectScreens();
  }
}

void ScreenGeometryManager::connectScreens() {
  const auto screens = QGuiApplication::screens();
  for (QScreen *screen : screens) {
    connect(screen, &QScreen::geometryChanged, this,
            &ScreenGeometryManager::handleGeometryChanged,
            Qt::UniqueConnection);
    connect(screen, &QScreen::physicalDotsPerInchChanged, this,
            &ScreenGeometryManager::screenTopologyChanged,
            Qt::UniqueConnection);
  }
}

void ScreenGeometryManager::handleScreenAdded(QScreen *screen) {
  if (screen) {
    connect(screen, &QScreen::geometryChanged, this,
            &ScreenGeometryManager::handleGeometryChanged,
            Qt::UniqueConnection);
    connect(screen, &QScreen::physicalDotsPerInchChanged, this,
            &ScreenGeometryManager::screenTopologyChanged,
            Qt::UniqueConnection);
  }
  emit screenTopologyChanged();
}

void ScreenGeometryManager::handleScreenRemoved(QScreen * /*screen*/) {
  emit screenTopologyChanged();
}

void ScreenGeometryManager::handleGeometryChanged(const QRect & /*geometry*/) {
  emit screenTopologyChanged();
}

QRect ScreenGeometryManager::totalVirtualGeometry() const {
  const auto screens = QGuiApplication::screens();
  if (screens.isEmpty()) {
    return {0, 0, 1920, 1080};
  }

  QRect total = screens.first()->geometry();
  for (int i = 1; i < screens.size(); ++i) {
    total = total.united(screens.at(i)->geometry());
  }
  return total;
}

QRect ScreenGeometryManager::totalPhysicalGeometry() const {
  const auto screens = QGuiApplication::screens();
  if (screens.isEmpty()) {
    return {0, 0, 1920, 1080};
  }

  const QRect virtualBounds = totalVirtualGeometry();
  int minX = 0;
  int minY = 0;
  int maxX = 0;
  int maxY = 0;

  for (QScreen *screen : screens) {
    const qreal dpr = screen->devicePixelRatio();
    const QRect geom = screen->geometry();
    const int px =
        static_cast<int>(std::round((geom.x() - virtualBounds.x()) * dpr));
    const int py =
        static_cast<int>(std::round((geom.y() - virtualBounds.y()) * dpr));
    const int pw = static_cast<int>(std::round(geom.width() * dpr));
    const int ph = static_cast<int>(std::round(geom.height() * dpr));

    minX = std::min(minX, px);
    minY = std::min(minY, py);
    maxX = std::max(maxX, px + pw);
    maxY = std::max(maxY, py + ph);
  }

  return {minX, minY, maxX - minX, maxY - minY};
}

QSize ScreenGeometryManager::totalVirtualSize() const {
  return totalVirtualGeometry().size();
}

QVector<ScreenInfo> ScreenGeometryManager::screenInfos() const {
  QVector<ScreenInfo> list;
  const auto screens = QGuiApplication::screens();
  QScreen *primary = QGuiApplication::primaryScreen();

  for (QScreen *screen : screens) {
    ScreenInfo info;
    info.name = screen->name();
    info.geometry = screen->geometry();
    const qreal dpr = screen->devicePixelRatio();
    info.devicePixelRatio = dpr;
    info.physicalGeometry =
        QRect(static_cast<int>(std::round(info.geometry.x() * dpr)),
              static_cast<int>(std::round(info.geometry.y() * dpr)),
              static_cast<int>(std::round(info.geometry.width() * dpr)),
              static_cast<int>(std::round(info.geometry.height() * dpr)));
    info.isPrimary = (screen == primary);
    list.append(info);
  }
  return list;
}

int ScreenGeometryManager::screenCount() const {
  return static_cast<int>(QGuiApplication::screens().size());
}

QScreen *ScreenGeometryManager::screenAt(const QPoint &pos) const {
  return QGuiApplication::screenAt(pos);
}

QScreen *ScreenGeometryManager::primaryScreen() const {
  return QGuiApplication::primaryScreen();
}

QScreen *ScreenGeometryManager::activeScreen() const {
  const QPoint cursorPos = QCursor::pos();
  QScreen *screen = QGuiApplication::screenAt(cursorPos);
  if (!screen) {
    screen = QGuiApplication::primaryScreen();
  }
  if (!screen) {
    const auto screens = QGuiApplication::screens();
    if (!screens.isEmpty()) {
      screen = screens.first();
    }
  }
  return screen;
}

QPoint
ScreenGeometryManager::logicalToVirtualCanvas(const QPoint &screenPos) const {
  const QRect bounds = totalVirtualGeometry();
  return {screenPos.x() - bounds.x(), screenPos.y() - bounds.y()};
}

QRect ScreenGeometryManager::logicalToVirtualCanvas(
    const QRect &screenRect) const {
  const QRect bounds = totalVirtualGeometry();
  return {screenRect.x() - bounds.x(), screenRect.y() - bounds.y(),
          screenRect.width(), screenRect.height()};
}

QPoint
ScreenGeometryManager::virtualCanvasToLogical(const QPoint &canvasPos) const {
  const QRect bounds = totalVirtualGeometry();
  return {canvasPos.x() + bounds.x(), canvasPos.y() + bounds.y()};
}

QRect ScreenGeometryManager::virtualCanvasToLogical(
    const QRect &canvasRect) const {
  const QRect bounds = totalVirtualGeometry();
  return {canvasRect.x() + bounds.x(), canvasRect.y() + bounds.y(),
          canvasRect.width(), canvasRect.height()};
}

QRect ScreenGeometryManager::mapOverlayToImage(const QRect &overlayRect,
                                               const QSize &overlaySize,
                                               const QSize &imageSize) const {
  if (overlaySize.isEmpty() || imageSize.isEmpty()) {
    return overlayRect;
  }

  const double scaleX =
      static_cast<double>(imageSize.width()) / overlaySize.width();
  const double scaleY =
      static_cast<double>(imageSize.height()) / overlaySize.height();

  const int x = static_cast<int>(std::round(overlayRect.x() * scaleX));
  const int y = static_cast<int>(std::round(overlayRect.y() * scaleY));
  const int w = static_cast<int>(std::round(overlayRect.width() * scaleX));
  const int h = static_cast<int>(std::round(overlayRect.height() * scaleY));

  const QRect result(x, y, w, h);
  return result.intersected(QRect(0, 0, imageSize.width(), imageSize.height()));
}

QRect ScreenGeometryManager::mapImageToOverlay(const QRect &imageRect,
                                               const QSize &imageSize,
                                               const QSize &overlaySize) const {
  if (overlaySize.isEmpty() || imageSize.isEmpty()) {
    return imageRect;
  }

  const double scaleX =
      static_cast<double>(overlaySize.width()) / imageSize.width();
  const double scaleY =
      static_cast<double>(overlaySize.height()) / imageSize.height();

  const int x = static_cast<int>(std::round(imageRect.x() * scaleX));
  const int y = static_cast<int>(std::round(imageRect.y() * scaleY));
  const int w = static_cast<int>(std::round(imageRect.width() * scaleX));
  const int h = static_cast<int>(std::round(imageRect.height() * scaleY));

  return QRect(x, y, w, h);
}

bool ScreenGeometryManager::isRegionValidInCurrentLayout(
    const QRect &region, const QSize &savedDesktopSize) const {
  if (!region.isValid() || region.width() <= 2 || region.height() <= 2) {
    return false;
  }
  if (!savedDesktopSize.isValid() || savedDesktopSize.isEmpty()) {
    return false;
  }

  const QSize currentSize = totalVirtualSize();
  if (currentSize != savedDesktopSize) {
    return false;
  }

  const QRect bounds(0, 0, currentSize.width(), currentSize.height());
  return bounds.contains(region);
}

QRect ScreenGeometryManager::sanitizeRegion(
    const QRect &region, const QSize &currentDesktopSize) const {
  QRect normalized = region.normalized();
  const QRect bounds(0, 0, currentDesktopSize.width(),
                     currentDesktopSize.height());
  QRect intersected = normalized.intersected(bounds);
  if (intersected.width() <= 2 || intersected.height() <= 2) {
    return {};
  }
  return intersected;
}

} // namespace ro_screenshot
