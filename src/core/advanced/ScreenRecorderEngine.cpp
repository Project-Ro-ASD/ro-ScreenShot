#include "ScreenRecorderEngine.hpp"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

namespace ro_screenshot {

ScreenRecorderEngine::ScreenRecorderEngine(QObject *parent)
    : QObject(parent), m_durationTimer(new QTimer(this)) {
  m_durationTimer->setInterval(1000);
  connect(m_durationTimer, &QTimer::timeout, this,
          &ScreenRecorderEngine::handleDurationTick);
}

ScreenRecorderEngine::~ScreenRecorderEngine() {
  if (m_state != RecordingState::Idle) {
    cancelRecording();
  }
}

bool ScreenRecorderEngine::isRecording() const {
  return m_state == RecordingState::Recording ||
         m_state == RecordingState::Paused;
}

bool ScreenRecorderEngine::isPaused() const {
  return m_state == RecordingState::Paused;
}

ScreenRecorderEngine::RecordingState ScreenRecorderEngine::state() const {
  return m_state;
}

int ScreenRecorderEngine::recordingDurationSeconds() const {
  return m_elapsedSeconds;
}

void ScreenRecorderEngine::handleDurationTick() {
  if (m_state == RecordingState::Recording) {
    m_elapsedSeconds++;
    emit durationChanged();
  }
}

bool ScreenRecorderEngine::startRecording(const RecordingOptions &options) {
  if (isRecording()) {
    return false;
  }

  m_options = options;
  m_elapsedSeconds = 0;
  m_state = RecordingState::Recording;
  m_durationTimer->start();

  emit stateChanged();
  emit durationChanged();

  // Recording pipeline hook (e.g. PipeWire / wf-recorder / ffmpeg / gstreamer)
  const QString wfRecorder =
      QStandardPaths::findExecutable(QStringLiteral("wf-recorder"));
  const QString ffmpeg =
      QStandardPaths::findExecutable(QStringLiteral("ffmpeg"));

  if (!wfRecorder.isEmpty() || !ffmpeg.isEmpty()) {
    m_recorderProcess = new QProcess(this);
    QString prog;
    QStringList args;

    if (!wfRecorder.isEmpty()) {
      prog = wfRecorder;
      if (options.targetRegion.isValid() && !options.targetRegion.isEmpty()) {
        args << QStringLiteral("-g")
             << QStringLiteral("%1,%2 %3x%4")
                    .arg(options.targetRegion.x())
                    .arg(options.targetRegion.y())
                    .arg(options.targetRegion.width())
                    .arg(options.targetRegion.height());
      }
      if (options.recordMic || options.recordSystemAudio) {
        args << QStringLiteral("-a");
      }
      args << QStringLiteral("-f") << options.outputPath;
    } else {
      prog = ffmpeg;
      args << QStringLiteral("-y") << QStringLiteral("-f")
           << QStringLiteral("x11grab") << QStringLiteral("-framerate")
           << QString::number(options.framerate) << QStringLiteral("-i")
           << QStringLiteral(":0.0") << options.outputPath;
    }

    m_recorderProcess->start(prog, args);
  }

  return true;
}

bool ScreenRecorderEngine::pauseRecording() {
  if (m_state != RecordingState::Recording) {
    return false;
  }
  m_state = RecordingState::Paused;
  m_durationTimer->stop();
  emit stateChanged();
  return true;
}

bool ScreenRecorderEngine::resumeRecording() {
  if (m_state != RecordingState::Paused) {
    return false;
  }
  m_state = RecordingState::Recording;
  m_durationTimer->start();
  emit stateChanged();
  return true;
}

bool ScreenRecorderEngine::stopRecording() {
  if (!isRecording()) {
    return false;
  }

  m_durationTimer->stop();
  m_state = RecordingState::Encoding;
  emit stateChanged();

  if (m_recorderProcess && m_recorderProcess->state() == QProcess::Running) {
    m_recorderProcess->terminate();
    if (!m_recorderProcess->waitForFinished(4000)) {
      m_recorderProcess->kill();
    }
    delete m_recorderProcess;
    m_recorderProcess = nullptr;
  }

  m_state = RecordingState::Idle;
  emit stateChanged();
  emit recordingFinished(m_options.outputPath, m_options.produceGif);
  return true;
}

void ScreenRecorderEngine::cancelRecording() {
  m_durationTimer->stop();
  if (m_recorderProcess) {
    m_recorderProcess->kill();
    delete m_recorderProcess;
    m_recorderProcess = nullptr;
  }
  if (!m_options.outputPath.isEmpty() && QFile::exists(m_options.outputPath)) {
    QFile::remove(m_options.outputPath);
  }
  m_state = RecordingState::Idle;
  emit stateChanged();
}

bool ScreenRecorderEngine::generateGifFromFrames(const QVector<QImage> &frames,
                                                 const QString &outputPath,
                                                 int fps) {
  if (frames.isEmpty() || outputPath.isEmpty()) {
    return false;
  }

  // Create temporary directory of frame PNGs
  const QString tempDir =
      QStandardPaths::writableLocation(QStandardPaths::CacheLocation) +
      "/gif_frames";
  QDir().mkpath(tempDir);

  for (int i = 0; i < frames.size(); ++i) {
    const QString framePath = QStringLiteral("%1/frame_%2.png")
                                  .arg(tempDir)
                                  .arg(i, 5, 10, QLatin1Char('0'));
    frames[i].save(framePath, "PNG");
  }

  const QString ffmpeg =
      QStandardPaths::findExecutable(QStringLiteral("ffmpeg"));
  if (ffmpeg.isEmpty()) {
    QDir(tempDir).removeRecursively();
    return false;
  }

  QProcess process;
  process.start(
      ffmpeg,
      QStringList{
          QStringLiteral("-y"), QStringLiteral("-framerate"),
          QString::number(fps > 0 ? fps : 15), QStringLiteral("-i"),
          tempDir + QStringLiteral("/frame_%05d.png"), QStringLiteral("-vf"),
          QStringLiteral("split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse"),
          outputPath});

  const bool ok = process.waitForFinished(15000) && process.exitCode() == 0;
  QDir(tempDir).removeRecursively();
  return ok;
}

} // namespace ro_screenshot
