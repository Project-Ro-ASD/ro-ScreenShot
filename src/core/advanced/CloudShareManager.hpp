#pragma once

#include "core/Types.hpp"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QObject>
#include <QString>
#include <QVariantMap>

namespace ro_screenshot {

struct UploadResult {
  bool success{false};
  QString directUrl;
  QString deleteUrl;
  QString errorMessage;
  CloudProviderType provider;
};

class ICloudUploader : public QObject {
  Q_OBJECT
public:
  using QObject::QObject;
  ~ICloudUploader() override = default;

  virtual CloudProviderType type() const = 0;
  virtual QString name() const = 0;
  virtual UploadResult upload(const QString &filePath,
                              const QVariantMap &config) = 0;
};

class SecureCredentialStore : public QObject {
  Q_OBJECT

public:
  explicit SecureCredentialStore(QObject *parent = nullptr);
  ~SecureCredentialStore() override = default;

  bool isSecretServiceAvailable() const;
  bool storeSecret(const QString &serviceName, const QString &account,
                   const QString &password);
  QString retrieveSecret(const QString &serviceName, const QString &account);
  bool deleteSecret(const QString &serviceName, const QString &account);

private:
  QString fallbackEncrypt(const QString &plain) const;
  QString fallbackDecrypt(const QString &encrypted) const;
};

class CloudShareManager : public QObject {
  Q_OBJECT

public:
  explicit CloudShareManager(QObject *parent = nullptr);
  ~CloudShareManager() override = default;

  QVector<CloudProviderType> availableProviders() const;
  UploadResult uploadScreenshot(const QString &filePath,
                                CloudProviderType provider,
                                const QVariantMap &options = {});

  // Secure credential interface (Secret Service / KWallet)
  bool saveProviderSecret(CloudProviderType provider, const QString &account,
                          const QString &secret);
  QString getProviderSecret(CloudProviderType provider, const QString &account);

signals:
  void uploadProgress(int percentage);
  void uploadCompleted(const UploadResult &result);

private:
  SecureCredentialStore m_secretStore;
};

} // namespace ro_screenshot
