#include "LivePresentationOverlay.hpp"
#include <QDateTime>

namespace ro_screenshot {

LivePresentationOverlay::LivePresentationOverlay(QObject *parent)
    : QObject(parent) {}

bool LivePresentationOverlay::isLaserActive() const { return m_laserActive; }

void LivePresentationOverlay::setLaserActive(bool active) {
  if (m_laserActive != active) {
    m_laserActive = active;
    emit laserActiveChanged(active);
    emit presentationUpdated();
  }
}

bool LivePresentationOverlay::isKeystrokeOverlayActive() const {
  return m_keystrokeActive;
}

void LivePresentationOverlay::setKeystrokeOverlayActive(bool active) {
  if (m_keystrokeActive != active) {
    m_keystrokeActive = active;
    emit keystrokeOverlayActiveChanged(active);
    emit presentationUpdated();
  }
}

QString LivePresentationOverlay::currentKeystroke() const {
  return m_currentKeystroke;
}

void LivePresentationOverlay::addLaserPoint(qreal x, qreal y) {
  if (!m_laserActive)
    return;

  LaserPoint pt;
  pt.pos = QPointF(x, y);
  pt.timestampMs = QDateTime::currentMSecsSinceEpoch();
  m_laserTrail.append(pt);
  pruneExpired();
  emit presentationUpdated();
}

void LivePresentationOverlay::recordKeystroke(const QString &keyCombo) {
  if (!m_keystrokeActive || keyCombo.isEmpty())
    return;

  m_currentKeystroke = keyCombo;
  KeystrokeBadge badge;
  badge.keySequence = keyCombo;
  badge.timestampMs = QDateTime::currentMSecsSinceEpoch();
  m_keystrokes.append(badge);

  emit currentKeystrokeChanged(keyCombo);
  emit presentationUpdated();
}

void LivePresentationOverlay::recordMouseClick(qreal /*x*/, qreal /*y*/,
                                               int button) {
  if (!m_keystrokeActive)
    return;

  QString btnStr = (button == 1)
                       ? QStringLiteral("Left Click")
                       : (button == 2 ? QStringLiteral("Right Click")
                                      : QStringLiteral("Middle Click"));
  recordKeystroke(btnStr);
}

void LivePresentationOverlay::pruneExpired(qint64 maxAgeMs) {
  qint64 now = QDateTime::currentMSecsSinceEpoch();

  // Prune laser trail
  for (qsizetype i = m_laserTrail.size() - 1; i >= 0; --i) {
    if (now - m_laserTrail[i].timestampMs > maxAgeMs) {
      m_laserTrail.remove(i);
    }
  }

  // Prune keystrokes
  for (qsizetype i = m_keystrokes.size() - 1; i >= 0; --i) {
    if (now - m_keystrokes[i].timestampMs > m_keystrokes[i].durationMs) {
      m_keystrokes.remove(i);
    }
  }

  if (m_keystrokes.isEmpty() && !m_currentKeystroke.isEmpty()) {
    m_currentKeystroke.clear();
    emit currentKeystrokeChanged(QString());
  }
}

} // namespace ro_screenshot
