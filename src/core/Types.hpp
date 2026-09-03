#pragma once

#include <QDateTime>
#include <QImage>
#include <QObject>
#include <QRect>
#include <QSize>
#include <QString>
#include <QVariantMap>

namespace ro_screenshot {
Q_NAMESPACE

enum class CaptureMode { Region, Fullscreen, Window };
Q_ENUM_NS(CaptureMode)

enum class CaptureAction { CopyAndSave, CopyOnly, SaveOnly };
Q_ENUM_NS(CaptureAction)

enum class ImageFormat { Png, Jpeg, WebP };
Q_ENUM_NS(ImageFormat)

enum class DateFilter { All, Today, Yesterday, ThisWeek, ThisMonth };
Q_ENUM_NS(DateFilter)

enum class CaptureErrorCode {
  None = 0,
  Cancelled = 1,
  PortalUnavailable = 2,
  PortalRejected = 3,
  Timeout = 4,
  InvalidImage = 5,
  DiskFull = 6,
  PermissionDenied = 7,
  EncoderFailed = 8,
  Busy = 9,
  Unknown = 10
};
Q_ENUM_NS(CaptureErrorCode)

enum class ColorFormat { Hex, Rgb, Hsl };
Q_ENUM_NS(ColorFormat)

enum class ShortcutAction {
  CaptureRegion,
  CaptureFullscreen,
  CaptureWindow,
  CaptureLastRegion,
  CaptureRegionCopyOnly,
  CaptureRegionSaveOnly,
  CaptureFullscreenDelayed,
  CaptureWindowDelayed,
  OpenGallery,
  OpenSettings,
  ColorPicker,
  ToggleRecording
};
Q_ENUM_NS(ShortcutAction)

enum class StructuredExitCode {
  Success = 0,
  GeneralError = 1,
  InvalidArguments = 2,
  CaptureCancelled = 3,
  CaptureFailed = 4,
  IoError = 5,
  DBusBusyError = 6
};
Q_ENUM_NS(StructuredExitCode)

enum class CloudProviderType { WebDAV, Nextcloud, S3, Imgur, CustomHttp };
Q_ENUM_NS(CloudProviderType)

struct ScreenshotItem {
  QString fileName;
  QString filePath;
  qint64 fileSize{0};
  QString formattedSize;
  QDateTime createdAt;
  QString formattedDate;
  QString resolution;
  int width{0};
  int height{0};
  QString thumbnailUrl;
};

struct CaptureResult {
  bool success{false};
  CaptureErrorCode errorCode{CaptureErrorCode::None};
  QString errorMessage;
  QString filePath;
  QImage image;
  CaptureMode mode{CaptureMode::Region};
  CaptureAction action{CaptureAction::CopyAndSave};
  QRect sourceRect;
  QSize virtualDesktopSize;
  QDateTime timestamp;
  bool savedToDisk{false};
  bool copiedToClipboard{false};
  int durationMs{0};
};

struct ColorSample {
  QString hex;
  QString rgb;
  QString hsl;
  int red{0};
  int green{0};
  int blue{0};
  int alpha{255};
};

struct ShortcutItem {
  ShortcutAction action;
  QString id;
  QString name;
  QString description;
  QString defaultKeySequence;
  QString currentKeySequence;
};

} // namespace ro_screenshot
