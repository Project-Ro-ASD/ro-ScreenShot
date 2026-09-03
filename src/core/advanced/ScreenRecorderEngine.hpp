#pragma once

#include <QImage>
#include <QObject>
#include <QProcess>
#include <QRect>
#include <QString>
#include <QTimer>
#include <QVector>

namespace ro_screenshot {

class ScreenRecorderEngine : public QObject {
  Q_OBJECT

public:
  enum class RecordingState { Idle, Recording, Paused, Encoding };
  Q_ENUM(RecordingState)

  struct RecordingOptions {
    bool recordMic{false};
    bool recordSystemAudio{false};
    bool cameraOverlay{false};
    bool produceGif{false};
    int framerate{30};
    int gifFramerate{15};
    QRect targetRegion;
    QString outputPath;
  };

  Q_PROPERTY(bool isRecording READ isRecording NOTIFY stateChanged)
  Q_PROPERTY(bool isPaused READ isPaused NOTIFY stateChanged)
  Q_PROPERTY(RecordingState state READ state NOTIFY stateChanged)
  Q_PROPERTY(int recordingDurationSeconds READ recordingDurationSeconds NOTIFY
                 durationChanged)

  explicit ScreenRecorderEngine(QObject *parent = nullptr);
  ~ScreenRecorderEngine() override;

  bool isRecording() const;
  bool isPaused() const;
  RecordingState state() const;
  int recordingDurationSeconds() const;

  bool startRecording(const RecordingOptions &options);
  bool pauseRecording();
  bool resumeRecording();
  bool stopRecording();
  void cancelRecording();

  // GIF Producer helper
  bool generateGifFromFrames(const QVector<QImage> &frames,
                             const QString &outputPath, int fps = 15);

signals:
  void stateChanged();
  void durationChanged();
  void recordingFinished(const QString &outputPath, bool isGif);
  void recordingFailed(const QString &errorMessage);

private slots:
  void handleDurationTick();

private:
  RecordingState m_state{RecordingState::Idle};
  RecordingOptions m_options;
  QTimer *m_durationTimer{nullptr};
  int m_elapsedSeconds{0};
  QProcess *m_recorderProcess{nullptr};
};

} // namespace ro_screenshot
