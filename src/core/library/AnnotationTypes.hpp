#pragma once

#include <QColor>
#include <QFont>
#include <QList>
#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <QUuid>

namespace ro_screenshot {

enum class ToolType {
  Select,
  Crop,
  Pen,
  Highlighter,
  Line,
  Arrow,
  Rectangle,
  Ellipse,
  Text,
  Blur,
  StepMarker,
  StatusBadge,
  CalloutLoupe
};

enum class StatusBadgeType {
  Checkmark, // Green ✓
  Cross,     // Red ✕
  Warning    // Yellow ⚠️
};

struct AnnotationItem {
  QString id;
  ToolType type{ToolType::Rectangle};
  QRectF rect;
  QList<QPointF> points;
  QColor strokeColor{Qt::red};
  QColor fillColor{Qt::transparent};
  qreal strokeWidth{3.0};
  QString text;
  int fontSize{14};
  int stepNumber{1};
  int blurRadius{10};
  StatusBadgeType badgeType{StatusBadgeType::Checkmark};
  qreal loupeZoom{2.0};
  qreal loupeRadius{36.0};
  bool isSelected{false};

  AnnotationItem() { id = QUuid::createUuid().toString(QUuid::WithoutBraces); }
};

} // namespace ro_screenshot
