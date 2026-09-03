#include "X11FallbackProvider.hpp"
#include <QGuiApplication>
#include <QPainter>
#include <QPixmap>
#include <QScreen>

namespace ro_screenshot {

X11FallbackProvider::X11FallbackProvider(ScreenGeometryManager *geometryManager,
                                         QObject *parent)
    : IScreenshotProvider(parent), m_geometryManager(geometryManager) {}

bool X11FallbackProvider::isAvailable() const {
  return !QGuiApplication::screens().isEmpty();
}

void X11FallbackProvider::capture(CaptureMode mode,
                                  const QVariantMap & /*options*/) {
  m_isCapturing = true;
  emit captureStarted(mode);

  QImage captured;
  QRect sourceRect;

  switch (mode) {
  case CaptureMode::Fullscreen:
  case CaptureMode::Region:
    captured = captureCombinedDesktop();
    if (!captured.isNull()) {
      sourceRect = captured.rect();
    }
    break;
  case CaptureMode::Window:
    captured = captureActiveWindow();
    if (!captured.isNull()) {
      sourceRect = captured.rect();
    }
    break;
  }

  m_isCapturing = false;

  if (captured.isNull()) {
    emit captureFailed(tr("Ekran görüntüsü yakalanamadı."),
                       CaptureErrorCode::InvalidImage);
    return;
  }

  emit captureReady(captured, sourceRect);
}

void X11FallbackProvider::cancel() {
  if (m_isCapturing) {
    m_isCapturing = false;
    emit captureCancelled();
  }
}

QImage X11FallbackProvider::captureCombinedDesktop() const {
  const auto screens = QGuiApplication::screens();
  if (screens.isEmpty()) {
    return {};
  }

  if (screens.size() == 1) {
    return screens.first()->grabWindow(0).toImage();
  }

  const QRect totalGeometry = m_geometryManager
                                  ? m_geometryManager->totalVirtualGeometry()
                                  : [screens]() {
                                      QRect t = screens.first()->geometry();
                                      for (int i = 1; i < screens.size(); ++i)
                                        t = t.united(screens[i]->geometry());
                                      return t;
                                    }();

  QImage combined(totalGeometry.size(), QImage::Format_ARGB32_Premultiplied);
  combined.fill(Qt::black);

  QPainter painter(&combined);
  for (QScreen *screen : screens) {
    QPixmap pix = screen->grabWindow(0);
    QRect geom = screen->geometry();
    painter.drawPixmap(geom.x() - totalGeometry.x(),
                       geom.y() - totalGeometry.y(), pix);
  }
  painter.end();

  return combined;
}

QImage X11FallbackProvider::captureActiveScreen() const {
  QScreen *screen = m_geometryManager ? m_geometryManager->activeScreen()
                                      : QGuiApplication::primaryScreen();
  if (!screen) {
    return {};
  }
  return screen->grabWindow(0).toImage();
}

QImage X11FallbackProvider::captureActiveWindow() const {
  // Grab from active screen
  return captureActiveScreen();
}

} // namespace ro_screenshot
