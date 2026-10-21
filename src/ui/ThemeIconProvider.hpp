#pragma once

#include <QFileInfo>
#include <QPainter>
#include <QQuickImageProvider>
#include <QSvgRenderer>

namespace ro_screenshot {

class ThemeIconProvider final : public QQuickImageProvider {
public:
  ThemeIconProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}

  QImage requestImage(const QString &id, QSize *size,
                      const QSize &requestedSize) override {
    const QString name = QFileInfo(id.section('/', 1)).fileName();
    const QColor tint(QStringLiteral("#") + id.section('/', 0, 0));
    if (!name.startsWith(QStringLiteral("icon-")) ||
        !name.endsWith(QStringLiteral(".svg")) || !tint.isValid())
      return {};
    QSvgRenderer renderer(QStringLiteral(":/qt/qml/ro_screenshot/assets/") +
                          name);
    if (!renderer.isValid())
      return {};
    const QSize dimensions =
        requestedSize.isValid() ? requestedSize : QSize(20, 20);
    QImage image(dimensions, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    renderer.render(&painter);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(image.rect(), tint);
    if (size)
      *size = image.size();
    return image;
  }
};

} // namespace ro_screenshot
