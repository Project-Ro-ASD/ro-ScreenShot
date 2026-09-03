#pragma once

#include <QImage>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QString>

namespace ro_screenshot {

enum class AiTaskType {
  ExplainError,
  ExtractCode,
  SummarizeText,
  TranslateContent
};

struct AiResponse {
  bool success{false};
  QString text;
  QString codeSnippet;
  QString errorMessage;
};

class AiAssistantEngine : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool isBusy READ isBusy NOTIFY busyChanged)
  Q_PROPERTY(QString apiKey READ apiKey WRITE setApiKey NOTIFY apiKeyChanged)
  Q_PROPERTY(
      QString endpoint READ endpoint WRITE setEndpoint NOTIFY endpointChanged)

public:
  explicit AiAssistantEngine(QObject *parent = nullptr);
  ~AiAssistantEngine() override = default;

  bool isBusy() const;
  QString apiKey() const;
  void setApiKey(const QString &key);

  QString endpoint() const;
  void setEndpoint(const QString &ep);

  Q_INVOKABLE void analyzeText(const QString &text, AiTaskType task);
  Q_INVOKABLE void analyzeImage(const QImage &image, AiTaskType task);

  // Synchronous heuristic local analyzer (offline fallback)
  static AiResponse analyzeOffline(const QString &text, AiTaskType task);

signals:
  void busyChanged(bool busy);
  void apiKeyChanged();
  void endpointChanged();
  void analysisReady(const AiResponse &response);
  void analysisFailed(const QString &errorMessage);

private:
  bool m_isBusy{false};
  QString m_apiKey;
  QString m_endpoint{"https://generativelanguage.googleapis.com/v1beta/models/"
                     "gemini-pro:generateContent"};
};

} // namespace ro_screenshot
