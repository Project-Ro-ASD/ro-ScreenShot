#pragma once

#include <QColor>
#include <QGradient>
#include <QImage>
#include <QObject>
#include <QString>

namespace ro_screenshot {

enum class MockupPreset {
  Sunset,
  Ocean,
  Cyberpunk,
  Velvet,
  Emerald,
  SlateDark,
  CustomSolid
};

struct MockupOptions {
  MockupPreset preset{MockupPreset::Sunset};
  int padding{48};
  int cornerRadius{14};
  int shadowBlur{30};
  int shadowOffsetY{12};
  QColor shadowColor{0, 0, 0, 110};
  QColor customColor1{QColor("#4F46E5")};
  QColor customColor2{QColor("#EC4899")};
  bool drawWindowControls{true};
  bool drawGlow{true};
};

class MockupFrameGenerator : public QObject {
  Q_OBJECT

public:
  explicit MockupFrameGenerator(QObject *parent = nullptr);
  ~MockupFrameGenerator() override = default;

  static QImage generate(const QImage &source, const MockupOptions &options);
  static QLinearGradient createPresetGradient(MockupPreset preset,
                                              const QRect &targetRect);
  static QString presetName(MockupPreset preset);
};

} // namespace ro_screenshot
