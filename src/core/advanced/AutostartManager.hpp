#pragma once

#include <QObject>
#include <QString>

namespace ro_screenshot {

class AutostartManager : public QObject {
  Q_OBJECT

public:
  explicit AutostartManager(QObject *parent = nullptr);
  ~AutostartManager() override = default;

  bool isAutostartEnabled() const;
  bool setAutostartEnabled(bool enabled);
  QString autostartFilePath() const;

private:
  QString m_desktopFileName{QStringLiteral("org.ro_asd.ro_screenshot.desktop")};
};

} // namespace ro_screenshot
