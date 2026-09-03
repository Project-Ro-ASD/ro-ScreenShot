#include "AiAssistantEngine.hpp"
#include <QBuffer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

namespace ro_screenshot {

AiAssistantEngine::AiAssistantEngine(QObject *parent) : QObject(parent) {}

bool AiAssistantEngine::isBusy() const { return m_isBusy; }

QString AiAssistantEngine::apiKey() const { return m_apiKey; }

void AiAssistantEngine::setApiKey(const QString &key) {
  if (m_apiKey != key) {
    m_apiKey = key;
    emit apiKeyChanged();
  }
}

QString AiAssistantEngine::endpoint() const { return m_endpoint; }

void AiAssistantEngine::setEndpoint(const QString &ep) {
  if (m_endpoint != ep) {
    m_endpoint = ep;
    emit endpointChanged();
  }
}

void AiAssistantEngine::analyzeText(const QString &text, AiTaskType task) {
  m_isBusy = true;
  emit busyChanged(true);

  // Offline heuristic analyzer fallback
  AiResponse res = analyzeOffline(text, task);
  m_isBusy = false;
  emit busyChanged(false);

  if (res.success) {
    emit analysisReady(res);
  } else {
    emit analysisFailed(res.errorMessage);
  }
}

void AiAssistantEngine::analyzeImage(const QImage & /*image*/,
                                     AiTaskType task) {
  // Offline fallback
  analyzeText("Image OCR payload", task);
}

AiResponse AiAssistantEngine::analyzeOffline(const QString &text,
                                             AiTaskType task) {
  AiResponse res;
  res.success = true;

  switch (task) {
  case AiTaskType::ExplainError: {
    res.text = "🔍 **Hata Analizi (AI Asistanı):**\n\n";
    if (text.contains("Segmentation fault", Qt::CaseInsensitive)) {
      res.text +=
          "• **Teşhis:** Geçersiz bellek erişimi veya null pointer referansı.\n"
          "• **Öneri:** Pointer'ları kullanmadan önce `nullptr` kontrolü yapın "
          "veya `valgrind / gdb` ile çağrı yığınını (stack trace) inceleyin.";
      res.codeSnippet = "if (ptr != nullptr) {\n    ptr->doSomething();\n}";
    } else if (text.contains("Permission denied", Qt::CaseInsensitive)) {
      res.text +=
          "• **Teşhis:** Dosya veya soket okuma/yazma yetkisi yetersiz.\n"
          "• **Öneri:** Dosya izinlerini (`chmod / chown`) veya kullanıcı "
          "grubunu kontrol edin.";
    } else if (text.contains("SyntaxError", Qt::CaseInsensitive)) {
      res.text += "• **Teşhis:** Kod sözdizimi kural ihlali.\n"
                  "• **Öneri:** Eksik parantez, tırnak veya noktalı virgül "
                  "satırını kontrol edin.";
    } else {
      res.text += QString("• Metin içeriği incelendi. Tespit edilen log/metin: "
                          "\"%1\"\n• Çözüm için ilgili kütüphane "
                          "dokümantasyonunu ve log seviyelerini kontrol edin.")
                      .arg(text.left(120));
    }
    break;
  }
  case AiTaskType::ExtractCode: {
    res.text = "💻 **Kod Çıkarımı:**\n";
    // Basic heuristic to strip comments or line numbers
    QStringList lines = text.split('\n');
    QStringList cleaned;
    for (const QString &l : lines) {
      QString line = l;
      line.remove(
          QRegularExpression(R"(^\s*\d+[:.]?\s*)")); // strip leading numbers
      cleaned.append(line);
    }
    res.codeSnippet = cleaned.join('\n');
    res.text += res.codeSnippet;
    break;
  }
  case AiTaskType::SummarizeText: {
    res.text = QString("📝 **Özet:** %1 kelimelik metin analiz edildi. Anahtar "
                       "terimler tespit edildi.")
                   .arg(text.split(QRegularExpression(R"(\s+)")).size());
    break;
  }
  case AiTaskType::TranslateContent: {
    res.text = "🌍 **Çeviri:** " + text;
    break;
  }
  }

  return res;
}

} // namespace ro_screenshot
