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
  Q_PROPERTY(
      QString formattedDuration READ formattedDuration NOTIFY durationChanged)
  Q_PROPERTY(double audioLevel READ audioLevel NOTIFY audioLevelChanged)
  Q_PROPERTY(
      bool micEnabled READ micEnabled WRITE setMicEnabled NOTIFY micChanged)
  Q_PROPERTY(bool systemAudioEnabled READ systemAudioEnabled WRITE
                 setSystemAudioEnabled NOTIFY systemAudioChanged)
  Q_PROPERTY(bool webcamPiPEnabled READ webcamPiPEnabled WRITE
                 setWebcamPiPEnabled NOTIFY webcamPiPChanged)

  explicit ScreenRecorderEngine(QObject *parent = nullptr);
  ~ScreenRecorderEngine() override;

  bool isRecording() const;
  bool isPaused() const;
  RecordingState state() const;
  int recordingDurationSeconds() const;
  QString formattedDuration() const;
  double audioLevel() const;

  bool micEnabled() const;
  void setMicEnabled(bool enabled);

  bool systemAudioEnabled() const;
  void setSystemAudioEnabled(bool enabled);

  bool webcamPiPEnabled() const;
  void setWebcamPiPEnabled(bool enabled);

  Q_INVOKABLE bool startRecording(const RecordingOptions &options);
  Q_INVOKABLE bool startRegionGif(const QRect &region, int durationSec = 5,
                                  const QString &outputPath = QString());
  Q_INVOKABLE bool pauseRecording();
  Q_INVOKABLE bool resumeRecording();
  Q_INVOKABLE bool stopRecording();
  Q_INVOKABLE void cancelRecording();

  // GIF Producer helper
  bool generateGifFromFrames(const QVector<QImage> &frames,
                             const QString &outputPath, int fps = 15);

signals:
  void stateChanged();
  void durationChanged();
  void audioLevelChanged();
  void micChanged();
  void systemAudioChanged();
  void webcamPiPChanged();
  void recordingFinished(const QString &outputPath, bool isGif);
  void recordingFailed(const QString &errorMessage);

private slots:
  void handleDurationTick();
  void handleAudioPulse();

private:
  RecordingState m_state{RecordingState::Idle};
  RecordingOptions m_options;
  QTimer *m_durationTimer{nullptr};
  QTimer *m_audioTimer{nullptr};
  int m_elapsedSeconds{0};
  double m_audioLevel{0.0};
  bool m_micEnabled{false};
  bool m_systemAudioEnabled{false};
  bool m_webcamPiPEnabled{false};
  QProcess *m_recorderProcess{nullptr};
};

} // namespace ro_screenshot
