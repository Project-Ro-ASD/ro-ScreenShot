#pragma once

#include <QGuiApplication>
#include <QObject>
#include <QPoint>
#include <QRect>
#include <QScreen>
#include <QSize>
#include <QVector>

namespace ro_screenshot {

struct ScreenInfo {
  QString name;
  QRect geometry;         // Logical geometry
  QRect physicalGeometry; // Physical pixel geometry
  qreal devicePixelRatio{1.0};
  bool isPrimary{false};
};

class ScreenGeometryManager : public QObject {
  Q_OBJECT

public:
  explicit ScreenGeometryManager(QObject *parent = nullptr);
  ~ScreenGeometryManager() override = default;

  // Virtual desktop geometry calculation
  QRect totalVirtualGeometry() const;
  QRect totalPhysicalGeometry() const;
  QSize totalVirtualSize() const;

  // Screen list & information
  QVector<ScreenInfo> screenInfos() const;
  int screenCount() const;
  QScreen *screenAt(const QPoint &pos) const;
  QScreen *primaryScreen() const;
  QScreen *activeScreen() const;

  // Multi-monitor & negative coordinate mappings
  QPoint logicalToVirtualCanvas(const QPoint &screenPos) const;
  QRect logicalToVirtualCanvas(const QRect &screenRect) const;
  QPoint virtualCanvasToLogical(const QPoint &canvasPos) const;
  QRect virtualCanvasToLogical(const QRect &canvasRect) const;

  // DPI / Coordinate Scaling transformations (Overlay <-> Captured Image)
  QRect mapOverlayToImage(const QRect &overlayRect, const QSize &overlaySize,
                          const QSize &imageSize) const;
  QRect mapImageToOverlay(const QRect &imageRect, const QSize &imageSize,
                          const QSize &overlaySize) const;

  // Region validation
  bool isRegionValidInCurrentLayout(const QRect &region,
                                    const QSize &savedDesktopSize) const;
  QRect sanitizeRegion(const QRect &region,
                       const QSize &currentDesktopSize) const;

signals:
  void screenTopologyChanged();

private slots:
  void handleScreenAdded(QScreen *screen);
  void handleScreenRemoved(QScreen *screen);
  void handleGeometryChanged(const QRect &geometry);

private:
  void connectScreens();
};

} // namespace ro_screenshot
