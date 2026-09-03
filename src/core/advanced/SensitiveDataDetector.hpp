#pragma once

#include <QList>
#include <QRect>
#include <QRegularExpression>
#include <QString>
#include <QVector>

namespace ro_screenshot {

enum class SensitiveDataType {
  TurkishId,
  Email,
  PhoneNumber,
  CreditCard,
  ApiKeyOrToken
};

struct SensitiveDataMatch {
  SensitiveDataType type;
  QString matchedText;
  int startIndex{0};
  int length{0};
  QString maskedText;
  QRect boundingBox;
};

class SensitiveDataDetector {
public:
  static bool validateTurkishId(const QString &id) {
    if (id.length() != 11)
      return false;
    if (id.startsWith('0'))
      return false;
    for (QChar c : id) {
      if (!c.isDigit())
        return false;
    }

    int digits[11];
    for (int i = 0; i < 11; ++i) {
      digits[i] = id[i].digitValue();
    }

    int oddSum = digits[0] + digits[2] + digits[4] + digits[6] + digits[8];
    int evenSum = digits[1] + digits[3] + digits[5] + digits[7];

    int d10 = ((oddSum * 7) - evenSum) % 10;
    if (d10 < 0)
      d10 += 10;
    if (d10 != digits[9])
      return false;

    int totalSum = 0;
    for (int i = 0; i < 10; ++i) {
      totalSum += digits[i];
    }
    if ((totalSum % 10) != digits[10])
      return false;

    return true;
  }

  static bool validateLuhnCreditCard(const QString &raw) {
    QString clean;
    for (QChar c : raw) {
      if (c.isDigit())
        clean.append(c);
    }
    if (clean.length() < 13 || clean.length() > 19)
      return false;

    int sum = 0;
    bool alternate = false;
    for (int i = clean.length() - 1; i >= 0; --i) {
      int n = clean[i].digitValue();
      if (alternate) {
        n *= 2;
        if (n > 9)
          n = (n % 10) + 1;
      }
      sum += n;
      alternate = !alternate;
    }
    return (sum % 10 == 0);
  }

  static QString maskValue(const QString &val, SensitiveDataType type) {
    if (val.isEmpty())
      return {};
    switch (type) {
    case SensitiveDataType::TurkishId:
      if (val.length() == 11) {
        return val.left(3) + "******" + val.right(2);
      }
      return "***********";
    case SensitiveDataType::Email: {
      int atIdx = val.indexOf('@');
      if (atIdx > 2) {
        return val.left(2) + "***" + val.mid(atIdx);
      }
      return "***@***";
    }
    case SensitiveDataType::PhoneNumber:
      if (val.length() >= 7) {
        return val.left(3) + "****" + val.right(2);
      }
      return "*******";
    case SensitiveDataType::CreditCard:
      if (val.length() >= 8) {
        return val.left(4) + " **** **** " + val.right(4);
      }
      return "**** **** **** ****";
    case SensitiveDataType::ApiKeyOrToken:
      if (val.length() >= 8) {
        return val.left(4) + "****************" + val.right(4);
      }
      return "****************";
    }
    return "******";
  }

  static QVector<SensitiveDataMatch> detect(const QString &text) {
    QVector<SensitiveDataMatch> matches;

    // 1. Turkish ID
    static const QRegularExpression tcknRegex(
        QStringLiteral(R"(\b[1-9]\d{10}\b)"));
    auto tcknIt = tcknRegex.globalMatch(text);
    while (tcknIt.hasNext()) {
      auto m = tcknIt.next();
      QString candidate = m.captured(0);
      if (validateTurkishId(candidate)) {
        SensitiveDataMatch match;
        match.type = SensitiveDataType::TurkishId;
        match.matchedText = candidate;
        match.startIndex = static_cast<int>(m.capturedStart(0));
        match.length = static_cast<int>(m.capturedLength(0));
        match.maskedText = maskValue(candidate, match.type);
        matches.append(match);
      }
    }

    // 2. Email
    static const QRegularExpression emailRegex(QStringLiteral(
        R"(\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,7}\b)"));
    auto emailIt = emailRegex.globalMatch(text);
    while (emailIt.hasNext()) {
      auto m = emailIt.next();
      QString candidate = m.captured(0);
      SensitiveDataMatch match;
      match.type = SensitiveDataType::Email;
      match.matchedText = candidate;
      match.startIndex = static_cast<int>(m.capturedStart(0));
      match.length = static_cast<int>(m.capturedLength(0));
      match.maskedText = maskValue(candidate, match.type);
      matches.append(match);
    }

    // 3. Phone Numbers
    static const QRegularExpression phoneRegex(QStringLiteral(
        R"(\b(?:\+?90|0)?\s*[1-9]\d{2}[\s.-]*\d{3}[\s.-]*\d{2}[\s.-]*\d{2}\b)"));
    auto phoneIt = phoneRegex.globalMatch(text);
    while (phoneIt.hasNext()) {
      auto m = phoneIt.next();
      QString candidate = m.captured(0);
      SensitiveDataMatch match;
      match.type = SensitiveDataType::PhoneNumber;
      match.matchedText = candidate;
      match.startIndex = static_cast<int>(m.capturedStart(0));
      match.length = static_cast<int>(m.capturedLength(0));
      match.maskedText = maskValue(candidate, match.type);
      matches.append(match);
    }

    // 4. Credit Cards
    static const QRegularExpression ccRegex(
        QStringLiteral(R"(\b(?:\d{4}[-\s]?){3}\d{4}\b)"));
    auto ccIt = ccRegex.globalMatch(text);
    while (ccIt.hasNext()) {
      auto m = ccIt.next();
      QString candidate = m.captured(0);
      if (validateLuhnCreditCard(candidate)) {
        SensitiveDataMatch match;
        match.type = SensitiveDataType::CreditCard;
        match.matchedText = candidate;
        match.startIndex = static_cast<int>(m.capturedStart(0));
        match.length = static_cast<int>(m.capturedLength(0));
        match.maskedText = maskValue(candidate, match.type);
        matches.append(match);
      }
    }

    // 5. API Keys / Tokens (GitHub, OpenAI, Google, AWS, Slack)
    static const QRegularExpression tokenRegex(QStringLiteral(
        R"(\b(?:ghp_[a-zA-Z0-9]{36}|AIza[0-9A-Za-z-_]{35}|sk-[a-zA-Z0-9]{20,48}|xox[baprs]-[0-9a-zA-Z]{10,48}|AKIA[0-9A-Z]{16})\b)"));
    auto tokenIt = tokenRegex.globalMatch(text);
    while (tokenIt.hasNext()) {
      auto m = tokenIt.next();
      QString candidate = m.captured(0);
      SensitiveDataMatch match;
      match.type = SensitiveDataType::ApiKeyOrToken;
      match.matchedText = candidate;
      match.startIndex = static_cast<int>(m.capturedStart(0));
      match.length = static_cast<int>(m.capturedLength(0));
      match.maskedText = maskValue(candidate, match.type);
      matches.append(match);
    }

    return matches;
  }
};

} // namespace ro_screenshot
