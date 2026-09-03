#pragma once

#include "core/library/AnnotationTypes.hpp"
#include <QList>
#include <QUndoCommand>

namespace ro_screenshot {

class AnnotationEngine;

class AddAnnotationCommand : public QUndoCommand {
public:
  AddAnnotationCommand(AnnotationEngine *engine, const AnnotationItem &item,
                       QUndoCommand *parent = nullptr);
  void undo() override;
  void redo() override;

private:
  AnnotationEngine *m_engine;
  AnnotationItem m_item;
};

class DeleteAnnotationCommand : public QUndoCommand {
public:
  DeleteAnnotationCommand(AnnotationEngine *engine, const QString &id,
                          QUndoCommand *parent = nullptr);
  void undo() override;
  void redo() override;

private:
  AnnotationEngine *m_engine;
  AnnotationItem m_item;
  int m_index{-1};
  bool m_hasItem{false};
};

class MoveAnnotationCommand : public QUndoCommand {
public:
  MoveAnnotationCommand(AnnotationEngine *engine, const QString &id, qreal dx,
                        qreal dy, QUndoCommand *parent = nullptr);
  void undo() override;
  void redo() override;

private:
  AnnotationEngine *m_engine;
  QString m_id;
  qreal m_dx;
  qreal m_dy;
};

class CropCommand : public QUndoCommand {
public:
  CropCommand(AnnotationEngine *engine, const QRectF &cropRect,
              QUndoCommand *parent = nullptr);
  void undo() override;
  void redo() override;

private:
  AnnotationEngine *m_engine;
  QRectF m_newCrop;
  QRectF m_oldCrop;
};

} // namespace ro_screenshot
