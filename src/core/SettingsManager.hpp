#pragma once

#include "Types.hpp"
#include <QDateTime>
#include <QObject>
#include <QRect>
#include <QSettings>
#include <QString>

namespace ro_screenshot {

class SettingsManager : public QObject {
  Q_OBJECT

  Q_PROPERTY(QString saveDirectory READ saveDirectory WRITE setSaveDirectory
                 NOTIFY saveDirectoryChanged)
  Q_PROPERTY(QString fileNameTemplate READ fileNameTemplate WRITE
                 setFileNameTemplate NOTIFY fileNameTemplateChanged)
  Q_PROPERTY(QString imageFormat READ imageFormat WRITE setImageFormat NOTIFY
                 imageFormatChanged)
  Q_PROPERTY(int jpegQuality READ jpegQuality WRITE setJpegQuality NOTIFY
                 jpegQualityChanged)
  Q_PROPERTY(bool autoCopyToClipboard READ autoCopyToClipboard WRITE
                 setAutoCopyToClipboard NOTIFY autoCopyToClipboardChanged)
  Q_PROPERTY(bool autoSaveToDisk READ autoSaveToDisk WRITE setAutoSaveToDisk
                 NOTIFY autoSaveToDiskChanged)
  Q_PROPERTY(bool showFloatingThumbnail READ showFloatingThumbnail WRITE
                 setShowFloatingThumbnail NOTIFY showFloatingThumbnailChanged)
  Q_PROPERTY(bool showNotification READ showNotification WRITE
                 setShowNotification NOTIFY showNotificationChanged)
  Q_PROPERTY(bool playShutterSound READ playShutterSound WRITE
                 setPlayShutterSound NOTIFY playShutterSoundChanged)
  Q_PROPERTY(
      bool createSubfoldersByMonth READ createSubfoldersByMonth WRITE
          setCreateSubfoldersByMonth NOTIFY createSubfoldersByMonthChanged)
  Q_PROPERTY(bool closeOverlayOnCapture READ closeOverlayOnCapture WRITE
                 setCloseOverlayOnCapture NOTIFY closeOverlayOnCaptureChanged)
  Q_PROPERTY(bool magnifierEnabled READ magnifierEnabled WRITE
                 setMagnifierEnabled NOTIFY magnifierEnabledChanged)
  Q_PROPERTY(int magnifierZoom READ magnifierZoom WRITE setMagnifierZoom NOTIFY
                 magnifierZoomChanged)
  Q_PROPERTY(QString previewFileName READ previewFileName NOTIFY
                 fileNameTemplateChanged)
  Q_PROPERTY(QRect lastRegion READ lastRegion WRITE setLastRegion NOTIFY
                 lastRegionChanged)

public:
  explicit SettingsManager(QObject *parent = nullptr);
  ~SettingsManager() override = default;

  QString saveDirectory() const;
  void setSaveDirectory(const QString &dir);

  QString fileNameTemplate() const;
  void setFileNameTemplate(const QString &tmpl);

  QString imageFormat() const;
  void setImageFormat(const QString &format);

  int jpegQuality() const;
  void setJpegQuality(int quality);

  bool autoCopyToClipboard() const;
  void setAutoCopyToClipboard(bool enable);

  bool autoSaveToDisk() const;
  void setAutoSaveToDisk(bool enable);

  bool showFloatingThumbnail() const;
  void setShowFloatingThumbnail(bool enable);

  bool showNotification() const;
  void setShowNotification(bool enable);

  bool playShutterSound() const;
  void setPlayShutterSound(bool enable);

  bool createSubfoldersByMonth() const;
  void setCreateSubfoldersByMonth(bool enable);

  bool closeOverlayOnCapture() const;
  void setCloseOverlayOnCapture(bool enable);

  bool magnifierEnabled() const;
  void setMagnifierEnabled(bool enable);

  int magnifierZoom() const;
  void setMagnifierZoom(int zoom);

  QRect lastRegion() const;
  void setLastRegion(const QRect &region);

  Q_INVOKABLE QString
  formatFileName(const QDateTime &dt = QDateTime::currentDateTime()) const;
  Q_INVOKABLE QString
  generateFullPath(const QDateTime &dt = QDateTime::currentDateTime()) const;
  Q_INVOKABLE QString previewFileName() const;
  Q_INVOKABLE void resetToDefaults();
  Q_INVOKABLE void sync();

signals:
  void saveDirectoryChanged();
  void fileNameTemplateChanged();
  void imageFormatChanged();
  void jpegQualityChanged();
  void autoCopyToClipboardChanged();
  void autoSaveToDiskChanged();
  void showFloatingThumbnailChanged();
  void showNotificationChanged();
  void playShutterSoundChanged();
  void createSubfoldersByMonthChanged();
  void closeOverlayOnCaptureChanged();
  void magnifierEnabledChanged();
  void magnifierZoomChanged();
  void lastRegionChanged();

private:
  void load();
  void ensureSaveDirectoryExists() const;

  QSettings m_settings;
  QString m_saveDirectory;
  QString m_fileNameTemplate;
  QString m_imageFormat;
  int m_jpegQuality{90};
  bool m_autoCopyToClipboard{true};
  bool m_autoSaveToDisk{true};
  bool m_showFloatingThumbnail{true};
  bool m_showNotification{true};
  bool m_playShutterSound{false};
  bool m_createSubfoldersByMonth{false};
  bool m_closeOverlayOnCapture{true};
  bool m_magnifierEnabled{true};
  int m_magnifierZoom{8};
  QRect m_lastRegion;
};

} // namespace ro_screenshot
