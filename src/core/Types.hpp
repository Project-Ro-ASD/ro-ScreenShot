#pragma once

#include <QDateTime>
#include <QImage>
#include <QObject>
#include <QRect>
#include <QString>

namespace ro_screenshot {
Q_NAMESPACE

enum class CaptureMode { Region, Fullscreen, Window };
Q_ENUM_NS(CaptureMode)

enum class ImageFormat { Png, Jpeg, WebP };
Q_ENUM_NS(ImageFormat)

enum class DateFilter { All, Today, Yesterday, ThisWeek, ThisMonth };
Q_ENUM_NS(DateFilter)

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
  QString errorMessage;
  QString filePath;
  QImage image;
  CaptureMode mode{CaptureMode::Region};
  QRect sourceRect;
  QDateTime timestamp;
};

} // namespace ro_screenshot
