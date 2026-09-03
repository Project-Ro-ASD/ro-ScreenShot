#pragma once

#include <QColor>
#include <QDateTime>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QVector>

namespace ro_screenshot {

struct LaserPoint {
  QPointF pos;
  qint64 timestampMs{0};
  qreal radius{8.0};
  QColor color{QColor("#EF4444")};
};

struct KeystrokeBadge {
  QString keySequence;
  qint64 timestampMs{0};
  int durationMs{2000};
  bool isMouseClick{false};
  QPointF clickPos;
};

class LivePresentationOverlay : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool isLaserActive READ isLaserActive WRITE setLaserActive NOTIFY
                 laserActiveChanged)
  Q_PROPERTY(bool isKeystrokeOverlayActive READ isKeystrokeOverlayActive WRITE
                 setKeystrokeOverlayActive NOTIFY keystrokeOverlayActiveChanged)
  Q_PROPERTY(QString currentKeystroke READ currentKeystroke NOTIFY
                 currentKeystrokeChanged)

public:
  explicit LivePresentationOverlay(QObject *parent = nullptr);
  ~LivePresentationOverlay() override = default;

  bool isLaserActive() const;
  void setLaserActive(bool active);

  bool isKeystrokeOverlayActive() const;
  void setKeystrokeOverlayActive(bool active);

  QString currentKeystroke() const;

  Q_INVOKABLE void addLaserPoint(qreal x, qreal y);
  Q_INVOKABLE void recordKeystroke(const QString &keyCombo);
  Q_INVOKABLE void recordMouseClick(qreal x, qreal y, int button = 1);
  Q_INVOKABLE void pruneExpired(qint64 maxAgeMs = 1500);

signals:
  void laserActiveChanged(bool active);
  void keystrokeOverlayActiveChanged(bool active);
  void currentKeystrokeChanged(const QString &key);
  void presentationUpdated();

private:
  bool m_laserActive{false};
  bool m_keystrokeActive{true};
  QString m_currentKeystroke;
  QVector<LaserPoint> m_laserTrail;
  QVector<KeystrokeBadge> m_keystrokes;
};

} // namespace ro_screenshot
