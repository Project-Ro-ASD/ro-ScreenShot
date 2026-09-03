#pragma once

#include "core/advanced/MockupFrameGenerator.hpp"
#include "core/advanced/OcrEngine.hpp"
#include "core/advanced/SensitiveDataDetector.hpp"
#include "core/library/AnnotationCommands.hpp"
#include "core/library/AnnotationTypes.hpp"
#include <QColor>
#include <QImage>
#include <QList>
#include <QPainter>
#include <QPainterPath>
#include <QRectF>
#include <QString>
#include <QUndoStack>
#include <cmath>
#include <functional>

namespace ro_screenshot {

class AnnotationEngine {
public:
  std::function<void()> onUndoRedoChanged;
  std::function<void()> onItemsChanged;
  std::function<void()> onImageLoaded;
  std::function<void()> onCurrentToolChanged;
  std::function<void()> onPropertiesChanged;
  std::function<void()> onStepNumberChanged;

  explicit AnnotationEngine() {
    QObject::connect(&m_undoStack, &QUndoStack::canUndoChanged, [this](bool) {
      if (onUndoRedoChanged) {
        onUndoRedoChanged();
      }
    });
    QObject::connect(&m_undoStack, &QUndoStack::canRedoChanged, [this](bool) {
      if (onUndoRedoChanged) {
        onUndoRedoChanged();
      }
    });
  }

  virtual ~AnnotationEngine() = default;

  bool canUndo() const { return m_undoStack.canUndo(); }
  bool canRedo() const { return m_undoStack.canRedo(); }
  int annotationCount() const { return static_cast<int>(m_items.size()); }

  int currentTool() const { return static_cast<int>(m_currentTool); }
  void setCurrentTool(int tool) {
    auto t = static_cast<ToolType>(tool);
    if (m_currentTool != t) {
      m_currentTool = t;
      emitCurrentToolChanged();
    }
  }

  QColor strokeColor() const { return m_strokeColor; }
  void setStrokeColor(const QColor &c) {
    if (m_strokeColor != c) {
      m_strokeColor = c;
      emitPropertiesChanged();
    }
  }

  qreal strokeWidth() const { return m_strokeWidth; }
  void setStrokeWidth(qreal w) {
    if (m_strokeWidth != w) {
      m_strokeWidth = w;
      emitPropertiesChanged();
    }
  }

  int nextStepNumber() const { return m_nextStepNumber; }

  void setBaseImage(const QImage &image) {
    m_baseImage = image;
    m_cropRect = QRectF(0, 0, static_cast<qreal>(image.width()),
                        static_cast<qreal>(image.height()));
    m_items.clear();
    m_undoStack.clear();
    m_nextStepNumber = 1;
    emitImageLoaded();
    emitItemsChanged();
    emitStepNumberChanged();
  }

  bool loadFromFile(const QString &path) {
    QImage img(path);
    if (img.isNull()) {
      return false;
    }
    setBaseImage(img);
    return true;
  }

  void undo() { m_undoStack.undo(); }
  void redo() { m_undoStack.redo(); }

  void addArrow(qreal x1, qreal y1, qreal x2, qreal y2) {
    AnnotationItem item;
    item.type = ToolType::Arrow;
    item.points = {QPointF(x1, y1), QPointF(x2, y2)};
    item.strokeColor = m_strokeColor;
    item.strokeWidth = m_strokeWidth;
    m_undoStack.push(new AddAnnotationCommand(this, item));
  }

  void addLine(qreal x1, qreal y1, qreal x2, qreal y2) {
    AnnotationItem item;
    item.type = ToolType::Line;
    item.points = {QPointF(x1, y1), QPointF(x2, y2)};
    item.strokeColor = m_strokeColor;
    item.strokeWidth = m_strokeWidth;
    m_undoStack.push(new AddAnnotationCommand(this, item));
  }

  void addRectangle(qreal x, qreal y, qreal w, qreal h, bool filled = false) {
    AnnotationItem item;
    item.type = ToolType::Rectangle;
    item.rect = QRectF(x, y, w, h);
    item.strokeColor = m_strokeColor;
    item.fillColor = filled ? QColor(m_strokeColor.red(), m_strokeColor.green(),
                                     m_strokeColor.blue(), 50)
                            : Qt::transparent;
    item.strokeWidth = m_strokeWidth;
    m_undoStack.push(new AddAnnotationCommand(this, item));
  }

  void addEllipse(qreal x, qreal y, qreal w, qreal h, bool filled = false) {
    AnnotationItem item;
    item.type = ToolType::Ellipse;
    item.rect = QRectF(x, y, w, h);
    item.strokeColor = m_strokeColor;
    item.fillColor = filled ? QColor(m_strokeColor.red(), m_strokeColor.green(),
                                     m_strokeColor.blue(), 50)
                            : Qt::transparent;
    item.strokeWidth = m_strokeWidth;
    m_undoStack.push(new AddAnnotationCommand(this, item));
  }

  void addPen(const QList<QPointF> &pts) {
    if (pts.isEmpty()) {
      return;
    }
    AnnotationItem item;
    item.type = ToolType::Pen;
    item.points = pts;
    item.strokeColor = m_strokeColor;
    item.strokeWidth = m_strokeWidth;
    m_undoStack.push(new AddAnnotationCommand(this, item));
  }

  void addHighlighter(const QList<QPointF> &pts) {
    if (pts.isEmpty()) {
      return;
    }
    AnnotationItem item;
    item.type = ToolType::Highlighter;
    item.points = pts;
    QColor c = m_strokeColor;
    c.setAlpha(100);
    item.strokeColor = c;
    item.strokeWidth = std::max<qreal>(m_strokeWidth * 3.0, 16.0);
    m_undoStack.push(new AddAnnotationCommand(this, item));
  }

  void addText(qreal x, qreal y, const QString &text, int fontSize = 16) {
    if (text.isEmpty()) {
      return;
    }
    AnnotationItem item;
    item.type = ToolType::Text;
    item.rect = QRectF(x, y, 200, 40);
    item.points = {QPointF(x, y)};
    item.text = text;
    item.fontSize = fontSize;
    item.strokeColor = m_strokeColor;
    m_undoStack.push(new AddAnnotationCommand(this, item));
  }

  void addBlur(qreal x, qreal y, qreal w, qreal h, int blurRadius = 12) {
    AnnotationItem item;
    item.type = ToolType::Blur;
    item.rect = QRectF(x, y, w, h);
    item.blurRadius = blurRadius;
    m_undoStack.push(new AddAnnotationCommand(this, item));
  }

  void addStepMarker(qreal x, qreal y) {
    AnnotationItem item;
    item.type = ToolType::StepMarker;
    item.points = {QPointF(x, y)};
    item.stepNumber = m_nextStepNumber++;
    item.strokeColor = m_strokeColor;
    emitStepNumberChanged();
    m_undoStack.push(new AddAnnotationCommand(this, item));
  }

  void addStatusBadge(qreal x, qreal y, StatusBadgeType badgeType) {
    AnnotationItem item;
    item.type = ToolType::StatusBadge;
    item.points = {QPointF(x, y)};
    item.badgeType = badgeType;
    switch (badgeType) {
    case StatusBadgeType::Checkmark:
      item.strokeColor = QColor("#22C55E");
      break; // Green
    case StatusBadgeType::Cross:
      item.strokeColor = QColor("#EF4444");
      break; // Red
    case StatusBadgeType::Warning:
      item.strokeColor = QColor("#F59E0B");
      break; // Amber
    }
    m_undoStack.push(new AddAnnotationCommand(this, item));
  }

  void addCalloutLoupe(qreal x, qreal y, qreal radius = 36.0,
                       qreal zoom = 2.0) {
    AnnotationItem item;
    item.type = ToolType::CalloutLoupe;
    item.points = {QPointF(x, y)};
    item.loupeRadius = radius;
    item.loupeZoom = zoom;
    item.strokeColor = m_strokeColor;
    item.strokeWidth = m_strokeWidth;
    m_undoStack.push(new AddAnnotationCommand(this, item));
  }

  int autoRedactSensitiveData(const QString &text,
                              const QVector<OcrTextBlock> &blocks) {
    auto matches = SensitiveDataDetector::detect(text);
    int added = 0;

    for (const auto &m : matches) {
      // Find matching OCR block bounds
      bool blockMatched = false;
      for (const auto &b : blocks) {
        if (b.text.contains(m.matchedText) || m.matchedText.contains(b.text)) {
          addBlur(b.boundingBox.x() - 4, b.boundingBox.y() - 4,
                  b.boundingBox.width() + 8, b.boundingBox.height() + 8);
          blockMatched = true;
          added++;
          break;
        }
      }
      if (!blockMatched && !blocks.isEmpty()) {
        // Fallback approximation
        addBlur(blocks[0].boundingBox.x(), blocks[0].boundingBox.y(), 120, 30);
        added++;
      }
    }
    return added;
  }

  void applyCrop(qreal x, qreal y, qreal w, qreal h) {
    QRectF newCrop(x, y, w, h);
    if (newCrop.isValid() && !newCrop.isEmpty()) {
      m_undoStack.push(new CropCommand(this, newCrop));
    }
  }

  void deleteAnnotation(const QString &id) {
    m_undoStack.push(new DeleteAnnotationCommand(this, id));
  }

  void moveAnnotation(const QString &id, qreal dx, qreal dy) {
    m_undoStack.push(new MoveAnnotationCommand(this, id, dx, dy));
  }

  void clearAll() {
    while (m_undoStack.canUndo()) {
      m_undoStack.undo();
    }
    m_items.clear();
    m_nextStepNumber = 1;
    emitItemsChanged();
    emitStepNumberChanged();
  }

  // Internal manipulation methods for Commands
  void internalAddItem(const AnnotationItem &item) {
    m_items.append(item);
    emitItemsChanged();
  }

  void internalRemoveItem(const QString &id) {
    for (int i = 0; i < m_items.size(); ++i) {
      if (m_items[i].id == id) {
        m_items.removeAt(i);
        emitItemsChanged();
        break;
      }
    }
  }

  void internalInsertItem(int index, const AnnotationItem &item) {
    if (index >= 0 && index <= m_items.size()) {
      m_items.insert(index, item);
    } else {
      m_items.append(item);
    }
    emitItemsChanged();
  }

  void internalMoveItem(const QString &id, qreal dx, qreal dy) {
    for (auto &item : m_items) {
      if (item.id == id) {
        if (!item.rect.isNull()) {
          item.rect.translate(dx, dy);
        }
        for (auto &p : item.points) {
          p += QPointF(dx, dy);
        }
        emitItemsChanged();
        break;
      }
    }
  }

  void internalSetCrop(const QRectF &crop) {
    m_cropRect = crop;
    emitItemsChanged();
  }

  QRectF currentCrop() const { return m_cropRect; }

  const QList<AnnotationItem> &items() const { return m_items; }

  QImage renderFlattened() const {
    if (m_baseImage.isNull()) {
      return {};
    }

    // 1. Crop base image
    QRect cropInt = m_cropRect.toRect().intersected(m_baseImage.rect());
    QImage cropped = m_baseImage.copy(cropInt);

    // 2. Apply blur/pixelate regions
    for (const auto &item : m_items) {
      if (item.type == ToolType::Blur) {
        QRect r = item.rect.translated(-m_cropRect.topLeft())
                      .toRect()
                      .intersected(cropped.rect());
        if (r.isValid() && !r.isEmpty()) {
          pixelateRegion(cropped, r, std::max(6, item.blurRadius));
        }
      }
    }

    // 3. Paint vector annotations
    QPainter painter(&cropped);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    for (const auto &item : m_items) {
      if (item.type == ToolType::Blur) {
        continue;
      }

      QPen pen(item.strokeColor, item.strokeWidth, Qt::SolidLine, Qt::RoundCap,
               Qt::RoundJoin);
      painter.setPen(pen);
      painter.setBrush(item.fillColor);

      QPointF offset = m_cropRect.topLeft();

      switch (item.type) {
      case ToolType::Rectangle: {
        QRectF r = item.rect.translated(-offset);
        painter.drawRoundedRect(r, 4, 4);
        break;
      }
      case ToolType::Ellipse: {
        QRectF r = item.rect.translated(-offset);
        painter.drawEllipse(r);
        break;
      }
      case ToolType::Line: {
        if (item.points.size() >= 2) {
          painter.drawLine(item.points[0] - offset, item.points[1] - offset);
        }
        break;
      }
      case ToolType::Arrow: {
        if (item.points.size() >= 2) {
          drawArrow(&painter, item.points[0] - offset, item.points[1] - offset,
                    item.strokeColor, item.strokeWidth);
        }
        break;
      }
      case ToolType::Pen: {
        if (item.points.size() > 1) {
          QPainterPath path;
          path.moveTo(item.points[0] - offset);
          for (int i = 1; i < item.points.size(); ++i) {
            path.lineTo(item.points[i] - offset);
          }
          painter.setBrush(Qt::NoBrush);
          painter.drawPath(path);
        }
        break;
      }
      case ToolType::Highlighter: {
        if (item.points.size() > 1) {
          painter.save();
          painter.setCompositionMode(QPainter::CompositionMode_Multiply);
          QPen highPen(item.strokeColor, item.strokeWidth, Qt::SolidLine,
                       Qt::RoundCap, Qt::RoundJoin);
          painter.setPen(highPen);
          painter.setBrush(Qt::NoBrush);
          QPainterPath path;
          path.moveTo(item.points[0] - offset);
          for (int i = 1; i < item.points.size(); ++i) {
            path.lineTo(item.points[i] - offset);
          }
          painter.drawPath(path);
          painter.restore();
        }
        break;
      }
      case ToolType::Text: {
        if (!item.points.isEmpty() && !item.text.isEmpty()) {
          painter.save();
          QFont font = painter.font();
          font.setPixelSize(item.fontSize);
          font.setBold(true);
          painter.setFont(font);
          painter.setPen(item.strokeColor);
          painter.drawText(item.points[0] - offset, item.text);
          painter.restore();
        }
        break;
      }
      case ToolType::StepMarker: {
        if (!item.points.isEmpty()) {
          QPointF center = item.points[0] - offset;
          qreal radius = 14.0;
          painter.setPen(Qt::NoPen);
          painter.setBrush(item.strokeColor);
          painter.drawEllipse(center, radius, radius);

          painter.setPen(Qt::white);
          QFont font = painter.font();
          font.setPixelSize(12);
          font.setBold(true);
          painter.setFont(font);
          QRectF numRect(center.x() - radius, center.y() - radius, radius * 2,
                         radius * 2);
          painter.drawText(numRect, Qt::AlignCenter,
                           QString::number(item.stepNumber));
        }
        break;
      }
      case ToolType::StatusBadge: {
        if (!item.points.isEmpty()) {
          QPointF center = item.points[0] - offset;
          qreal radius = 16.0;
          painter.setPen(Qt::NoPen);
          painter.setBrush(item.strokeColor);
          painter.drawEllipse(center, radius, radius);

          painter.setPen(Qt::white);
          QFont font = painter.font();
          font.setPixelSize(14);
          font.setBold(true);
          painter.setFont(font);
          QRectF badgeRect(center.x() - radius, center.y() - radius, radius * 2,
                           radius * 2);

          QString sym = "✓";
          if (item.badgeType == StatusBadgeType::Cross)
            sym = "✕";
          else if (item.badgeType == StatusBadgeType::Warning)
            sym = "!";
          painter.drawText(badgeRect, Qt::AlignCenter, sym);
        }
        break;
      }
      case ToolType::CalloutLoupe: {
        if (!item.points.isEmpty()) {
          QPointF center = item.points[0] - offset;
          qreal r = item.loupeRadius > 0 ? item.loupeRadius : 36.0;
          qreal zoom = item.loupeZoom > 1.0 ? item.loupeZoom : 2.0;

          // Source area around center in uncropped coordinates
          qreal srcW = (r * 2.0) / zoom;
          qreal srcH = (r * 2.0) / zoom;
          QRectF srcRect(item.points[0].x() - srcW / 2.0,
                         item.points[0].y() - srcH / 2.0, srcW, srcH);
          QImage subImg =
              m_baseImage.copy(srcRect.toRect())
                  .scaled(static_cast<int>(r * 2), static_cast<int>(r * 2),
                          Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

          painter.save();
          QPainterPath clip;
          clip.addEllipse(center, r, r);
          painter.setClipPath(clip);
          painter.drawImage(
              QRectF(center.x() - r, center.y() - r, r * 2, r * 2), subImg);
          painter.restore();

          // Loupe ring
          painter.setPen(QPen(item.strokeColor,
                              item.strokeWidth > 0 ? item.strokeWidth : 3.0));
          painter.setBrush(Qt::NoBrush);
          painter.drawEllipse(center, r, r);
        }
        break;
      }
      default:
        break;
      }
    }

    painter.end();
    return cropped;
  }

  QImage renderWithMockupFrame(const MockupOptions &options) const {
    QImage flat = renderFlattened();
    if (flat.isNull())
      return {};
    return MockupFrameGenerator::generate(flat, options);
  }

  bool exportToFile(const QString &destPath, const QString &format = "PNG") {
    QImage rendered = renderFlattened();
    if (rendered.isNull()) {
      return false;
    }
    return rendered.save(destPath, format.toLatin1().constData());
  }

private:
  void emitUndoRedoChanged() {
    if (onUndoRedoChanged) {
      onUndoRedoChanged();
    }
  }

  void emitItemsChanged() {
    if (onItemsChanged) {
      onItemsChanged();
    }
  }

  void emitImageLoaded() {
    if (onImageLoaded) {
      onImageLoaded();
    }
  }

  void emitCurrentToolChanged() {
    if (onCurrentToolChanged) {
      onCurrentToolChanged();
    }
  }

  void emitPropertiesChanged() {
    if (onPropertiesChanged) {
      onPropertiesChanged();
    }
  }

  void emitStepNumberChanged() {
    if (onStepNumberChanged) {
      onStepNumberChanged();
    }
  }

  static void pixelateRegion(QImage &img, const QRect &r, int blockSize) {
    for (int y = r.top(); y <= r.bottom(); y += blockSize) {
      for (int x = r.left(); x <= r.right(); x += blockSize) {
        int blockW = std::min(blockSize, r.right() - x + 1);
        int blockH = std::min(blockSize, r.bottom() - y + 1);

        qint64 rSum = 0, gSum = 0, bSum = 0, count = 0;
        for (int by = 0; by < blockH; ++by) {
          for (int bx = 0; bx < blockW; ++bx) {
            QRgb p = img.pixel(x + bx, y + by);
            rSum += qRed(p);
            gSum += qGreen(p);
            bSum += qBlue(p);
            count++;
          }
        }
        if (count > 0) {
          QRgb avg = qRgb(static_cast<int>(rSum / count),
                          static_cast<int>(gSum / count),
                          static_cast<int>(bSum / count));
          for (int by = 0; by < blockH; ++by) {
            for (int bx = 0; bx < blockW; ++bx) {
              img.setPixel(x + bx, y + by, avg);
            }
          }
        }
      }
    }
  }

  static void drawArrow(QPainter *painter, const QPointF &start,
                        const QPointF &end, const QColor &color, qreal width) {
    painter->save();
    QPen pen(color, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(pen);
    painter->drawLine(start, end);

    // Draw arrowhead
    double angle = std::atan2(end.y() - start.y(), end.x() - start.x());
    double headLength = std::max(12.0, width * 3.5);
    double headAngle = 0.45; // ~25 degrees

    QPointF p1 = end - QPointF(headLength * std::cos(angle - headAngle),
                               headLength * std::sin(angle - headAngle));
    QPointF p2 = end - QPointF(headLength * std::cos(angle + headAngle),
                               headLength * std::sin(angle + headAngle));

    QPainterPath headPath;
    headPath.moveTo(end);
    headPath.lineTo(p1);
    headPath.lineTo(p2);
    headPath.closeSubpath();

    painter->setBrush(color);
    painter->drawPath(headPath);
    painter->restore();
  }

  QImage m_baseImage;
  QRectF m_cropRect;
  QList<AnnotationItem> m_items;
  QUndoStack m_undoStack;
  ToolType m_currentTool{ToolType::Select};
  QColor m_strokeColor{QColor("#EF4444")}; // Red default
  qreal m_strokeWidth{3.0};
  int m_nextStepNumber{1};
};

inline AddAnnotationCommand::AddAnnotationCommand(AnnotationEngine *engine,
                                                  const AnnotationItem &item,
                                                  QUndoCommand *parent)
    : QUndoCommand(parent), m_engine(engine), m_item(item) {}

inline void AddAnnotationCommand::redo() {
  if (m_engine) {
    m_engine->internalAddItem(m_item);
  }
}

inline void AddAnnotationCommand::undo() {
  if (m_engine) {
    m_engine->internalRemoveItem(m_item.id);
  }
}

inline DeleteAnnotationCommand::DeleteAnnotationCommand(
    AnnotationEngine *engine, const QString &id, QUndoCommand *parent)
    : QUndoCommand(parent), m_engine(engine) {
  if (m_engine) {
    const auto &items = m_engine->items();
    for (int i = 0; i < items.size(); ++i) {
      if (items[i].id == id) {
        m_item = items[i];
        m_index = i;
        m_hasItem = true;
        break;
      }
    }
  }
}

inline void DeleteAnnotationCommand::redo() {
  if (m_engine && m_hasItem) {
    m_engine->internalRemoveItem(m_item.id);
  }
}

inline void DeleteAnnotationCommand::undo() {
  if (m_engine && m_hasItem) {
    m_engine->internalInsertItem(m_index, m_item);
  }
}

inline MoveAnnotationCommand::MoveAnnotationCommand(AnnotationEngine *engine,
                                                    const QString &id, qreal dx,
                                                    qreal dy,
                                                    QUndoCommand *parent)
    : QUndoCommand(parent), m_engine(engine), m_id(id), m_dx(dx), m_dy(dy) {}

inline void MoveAnnotationCommand::redo() {
  if (m_engine) {
    m_engine->internalMoveItem(m_id, m_dx, m_dy);
  }
}

inline void MoveAnnotationCommand::undo() {
  if (m_engine) {
    m_engine->internalMoveItem(m_id, -m_dx, -m_dy);
  }
}

inline CropCommand::CropCommand(AnnotationEngine *engine,
                                const QRectF &cropRect, QUndoCommand *parent)
    : QUndoCommand(parent), m_engine(engine), m_newCrop(cropRect) {
  if (m_engine) {
    m_oldCrop = m_engine->currentCrop();
  }
}

inline void CropCommand::redo() {
  if (m_engine) {
    m_engine->internalSetCrop(m_newCrop);
  }
}

inline void CropCommand::undo() {
  if (m_engine) {
    m_engine->internalSetCrop(m_oldCrop);
  }
}

} // namespace ro_screenshot
