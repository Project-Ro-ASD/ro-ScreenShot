#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QImage>
#include <QObject>
#include <QString>
#include <QVector>

namespace ro_screenshot {

struct VaultItem {
  QString id;
  QString originalFileName;
  QString vaultFilePath;
  qint64 fileSize{0};
  QDateTime encryptedAt;
  QString previewDataUrl;
};

class VaultManager : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool isUnlocked READ isUnlocked NOTIFY unlockStateChanged)
  Q_PROPERTY(int vaultItemCount READ vaultItemCount NOTIFY vaultChanged)
  Q_PROPERTY(QString vaultDirectory READ vaultDirectory WRITE setVaultDirectory
                 NOTIFY vaultDirectoryChanged)

public:
  explicit VaultManager(QObject *parent = nullptr);
  ~VaultManager() override = default;

  bool isUnlocked() const;
  int vaultItemCount() const;
  QString vaultDirectory() const;
  void setVaultDirectory(const QString &dir);

  Q_INVOKABLE bool unlock(const QString &password);
  Q_INVOKABLE void lock();
  Q_INVOKABLE bool isPasswordSet() const;
  Q_INVOKABLE bool setInitialPassword(const QString &password);
  Q_INVOKABLE bool changePassword(const QString &oldPassword,
                                  const QString &newPassword);

  Q_INVOKABLE bool encryptAndStoreImage(const QString &sourceImagePath,
                                        const QString &password = QString());
  Q_INVOKABLE QImage decryptImage(const QString &vaultFilePath,
                                  const QString &password = QString());
  Q_INVOKABLE bool exportDecryptedImage(const QString &vaultFilePath,
                                        const QString &destinationPath);
  Q_INVOKABLE bool deleteVaultItem(const QString &vaultFilePath);
  Q_INVOKABLE QVector<VaultItem> listVaultItems();

signals:
  void unlockStateChanged(bool unlocked);
  void vaultChanged();
  void vaultDirectoryChanged();
  void vaultError(const QString &message);

private:
  static QByteArray deriveKey(const QString &password, const QByteArray &salt,
                              int iterations = 10000);
  static QByteArray encryptAes256(const QByteArray &plainData,
                                  const QByteArray &key, const QByteArray &iv);
  static QByteArray decryptAes256(const QByteArray &cipherData,
                                  const QByteArray &key, const QByteArray &iv);

  QString m_vaultDir;
  QString m_cachedMasterPassword;
  bool m_unlocked{false};
};

} // namespace ro_screenshot
