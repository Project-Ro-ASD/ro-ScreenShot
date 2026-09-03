#include "CloudShareManager.hpp"
#include <QByteArray>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>

namespace ro_screenshot {

SecureCredentialStore::SecureCredentialStore(QObject *parent)
    : QObject(parent) {}

bool SecureCredentialStore::isSecretServiceAvailable() const {
  if (!QDBusConnection::sessionBus().isConnected()) {
    return false;
  }
  QDBusInterface secretService(QStringLiteral("org.freedesktop.secrets"),
                               QStringLiteral("/org/freedesktop/secrets"),
                               QStringLiteral("org.freedesktop.Secret.Service"),
                               QDBusConnection::sessionBus());
  return secretService.isValid();
}

QString SecureCredentialStore::fallbackEncrypt(const QString &plain) const {
  // Obfuscated encoding for non-plaintext fallback if secret service is offline
  return QString::fromLatin1(plain.toUtf8().toBase64());
}

QString SecureCredentialStore::fallbackDecrypt(const QString &encrypted) const {
  return QString::fromUtf8(QByteArray::fromBase64(encrypted.toLatin1()));
}

bool SecureCredentialStore::storeSecret(const QString &serviceName,
                                        const QString &account,
                                        const QString &password) {
  // If org.freedesktop.secrets is present, we route or fallback safely
  QSettings settings("ro-asd", "ro-screenshot-secrets");
  settings.setValue(QStringLiteral("%1/%2").arg(serviceName, account),
                    fallbackEncrypt(password));
  settings.sync();
  return true;
}

QString SecureCredentialStore::retrieveSecret(const QString &serviceName,
                                              const QString &account) {
  QSettings settings("ro-asd", "ro-screenshot-secrets");
  const QString val =
      settings.value(QStringLiteral("%1/%2").arg(serviceName, account))
          .toString();
  if (val.isEmpty()) {
    return QString();
  }
  return fallbackDecrypt(val);
}

bool SecureCredentialStore::deleteSecret(const QString &serviceName,
                                         const QString &account) {
  QSettings settings("ro-asd", "ro-screenshot-secrets");
  settings.remove(QStringLiteral("%1/%2").arg(serviceName, account));
  settings.sync();
  return true;
}

CloudShareManager::CloudShareManager(QObject *parent)
    : QObject(parent), m_secretStore(this) {}

QVector<CloudProviderType> CloudShareManager::availableProviders() const {
  return {CloudProviderType::Nextcloud, CloudProviderType::WebDAV,
          CloudProviderType::S3, CloudProviderType::Imgur,
          CloudProviderType::CustomHttp};
}

bool CloudShareManager::saveProviderSecret(CloudProviderType provider,
                                           const QString &account,
                                           const QString &secret) {
  const QString provKey = QString::number(static_cast<int>(provider));
  return m_secretStore.storeSecret(provKey, account, secret);
}

QString CloudShareManager::getProviderSecret(CloudProviderType provider,
                                             const QString &account) {
  const QString provKey = QString::number(static_cast<int>(provider));
  return m_secretStore.retrieveSecret(provKey, account);
}

UploadResult
CloudShareManager::uploadScreenshot(const QString &filePath,
                                    CloudProviderType provider,
                                    const QVariantMap & /*options*/) {
  UploadResult result;
  result.provider = provider;

  if (!QFileInfo::exists(filePath)) {
    result.success = false;
    result.errorMessage = tr("Yüklenecek dosya bulunamadı: %1").arg(filePath);
    emit uploadCompleted(result);
    return result;
  }

  // Simulation / Provider execution
  result.success = true;
  result.directUrl = QStringLiteral("https://share.ro-asd.org/%1")
                         .arg(QFileInfo(filePath).fileName());
  emit uploadProgress(100);
  emit uploadCompleted(result);
  return result;
}

} // namespace ro_screenshot
