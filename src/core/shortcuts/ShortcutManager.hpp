#pragma once

#include "core/Types.hpp"
#include <QKeySequence>
#include <QMap>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QVector>

namespace ro_screenshot {

class ShortcutManager : public QObject {
  Q_OBJECT

public:
  explicit ShortcutManager(QObject *parent = nullptr);
  ~ShortcutManager() override = default;

  // Shortcut definition list
  QVector<ShortcutItem> shortcuts() const;
  ShortcutItem shortcut(ShortcutAction action) const;
  QString keySequence(ShortcutAction action) const;
  bool setKeySequence(ShortcutAction action, const QString &keySequence);

  // Conflict detection
  bool hasConflict(ShortcutAction action, const QString &candidateSequence,
                   ShortcutAction *conflictingAction = nullptr) const;
  QVector<QPair<ShortcutAction, ShortcutAction>> allConflicts() const;

  // Reset
  void resetToDefaults();
  void resetShortcut(ShortcutAction action);

  // Exporters / Config Generators for Linux Desktop Environments & Window
  // Managers
  QString generateKdeShortcutsConfig() const;
  QString generateGnomeShortcutsScript() const;
  QString generateHyprlandConfig() const;
  QString generateSwayConfig() const;

  // Settings persistence
  void load();
  void save();

signals:
  void shortcutsChanged();

private:
  void initDefaultDefinitions();

  QMap<ShortcutAction, ShortcutItem> m_shortcuts;
  QSettings m_settings;
};

} // namespace ro_screenshot
