#pragma once

#include <QImage>
#include <QObject>
#include <QVector>

namespace ro_screenshot {

class ScrollingCaptureEngine : public QObject {
  Q_OBJECT

public:
  explicit ScrollingCaptureEngine(QObject *parent = nullptr);
  ~ScrollingCaptureEngine() override = default;

  // Stitches a sequence of vertically scrolled screenshot frames into a single
  // continuous tall image
  static QImage stitchFrames(const QVector<QImage> &frames,
                             int maxOverlapSearch = 300);

  // Finds the vertical offset between two overlapping frames
  static int findVerticalOverlap(const QImage &topFrame,
                                 const QImage &bottomFrame,
                                 int maxSearch = 300);
};

} // namespace ro_screenshot
