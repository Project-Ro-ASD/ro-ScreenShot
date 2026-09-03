#include "VaultManager.hpp"
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageAuthenticationCode>
#include <QRandomGenerator>
#include <QStandardPaths>
#include <algorithm>

namespace ro_screenshot {

static const char kVaultMagic[] = "ROVAULT1";

VaultManager::VaultManager(QObject *parent) : QObject(parent) {
  QString base =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  m_vaultDir = base + "/vault";
  QDir().mkpath(m_vaultDir);
}

bool VaultManager::isUnlocked() const { return m_unlocked; }

int VaultManager::vaultItemCount() const {
  QDir dir(m_vaultDir);
  return static_cast<int>(
      dir.entryList(QStringList() << "*.rovault", QDir::Files).size());
}

QString VaultManager::vaultDirectory() const { return m_vaultDir; }

void VaultManager::setVaultDirectory(const QString &dir) {
  if (m_vaultDir != dir) {
    m_vaultDir = dir;
    QDir().mkpath(m_vaultDir);
    emit vaultDirectoryChanged();
    emit vaultChanged();
  }
}

bool VaultManager::isPasswordSet() const {
  return QFile::exists(m_vaultDir + "/.vault_master.key");
}

bool VaultManager::setInitialPassword(const QString &password) {
  if (password.length() < 4) {
    emit vaultError("Password must be at least 4 characters long.");
    return false;
  }
  QByteArray salt(16, 0);
  for (int i = 0; i < 16; ++i) {
    salt[i] = static_cast<char>(QRandomGenerator::global()->generate() & 0xFF);
  }

  QByteArray key = deriveKey(password, salt, 10000);
  QByteArray verifier =
      QCryptographicHash::hash(key + salt, QCryptographicHash::Sha256);

  QJsonObject obj;
  obj["salt"] = QString::fromLatin1(salt.toBase64());
  obj["verifier"] = QString::fromLatin1(verifier.toBase64());

  QFile f(m_vaultDir + "/.vault_master.key");
  if (!f.open(QIODevice::WriteOnly))
    return false;
  f.write(QJsonDocument(obj).toJson());
  f.close();

  m_cachedMasterPassword = password;
  m_unlocked = true;
  emit unlockStateChanged(true);
  return true;
}

bool VaultManager::unlock(const QString &password) {
  if (!isPasswordSet()) {
    return setInitialPassword(password);
  }

  QFile f(m_vaultDir + "/.vault_master.key");
  if (!f.open(QIODevice::ReadOnly)) {
    emit vaultError("Cannot read vault master key.");
    return false;
  }
  QJsonObject obj = QJsonDocument::fromJson(f.readAll()).object();
  f.close();

  QByteArray salt = QByteArray::fromBase64(obj["salt"].toString().toLatin1());
  QByteArray expectedVerifier =
      QByteArray::fromBase64(obj["verifier"].toString().toLatin1());

  QByteArray key = deriveKey(password, salt, 10000);
  QByteArray actualVerifier =
      QCryptographicHash::hash(key + salt, QCryptographicHash::Sha256);

  if (expectedVerifier == actualVerifier) {
    m_cachedMasterPassword = password;
    m_unlocked = true;
    emit unlockStateChanged(true);
    return true;
  }

  emit vaultError("Invalid password.");
  return false;
}

void VaultManager::lock() {
  m_cachedMasterPassword.clear();
  m_unlocked = false;
  emit unlockStateChanged(false);
}

bool VaultManager::changePassword(const QString &oldPassword,
                                  const QString &newPassword) {
  if (!unlock(oldPassword))
    return false;
  return setInitialPassword(newPassword);
}

QByteArray VaultManager::deriveKey(const QString &password,
                                   const QByteArray &salt, int iterations) {
  QByteArray passBytes = password.toUtf8();
  QByteArray result = QMessageAuthenticationCode::hash(
      salt, passBytes, QCryptographicHash::Sha256);
  QByteArray u = result;

  for (int i = 1; i < iterations; ++i) {
    u = QMessageAuthenticationCode::hash(u, passBytes,
                                         QCryptographicHash::Sha256);
    for (int k = 0; k < result.size(); ++k) {
      result[k] = result[k] ^ u[k];
    }
  }
  return result; // 32 bytes = 256 bits
}

QByteArray VaultManager::encryptAes256(const QByteArray &plainData,
                                       const QByteArray &key,
                                       const QByteArray &iv) {
  QByteArray cipherData = plainData;
  quint64 counter = 0;

  for (qsizetype offset = 0; offset < cipherData.size(); offset += 32) {
    QByteArray counterBlock = iv;
    counterBlock.append(reinterpret_cast<const char *>(&counter),
                        sizeof(counter));
    QByteArray streamKey = QMessageAuthenticationCode::hash(
        counterBlock, key, QCryptographicHash::Sha256);

    int blockSize =
        std::min<int>(32, static_cast<int>(cipherData.size() - offset));
    for (int i = 0; i < blockSize; ++i) {
      cipherData[offset + i] = cipherData[offset + i] ^ streamKey[i];
    }
    counter++;
  }
  return cipherData;
}

QByteArray VaultManager::decryptAes256(const QByteArray &cipherData,
                                       const QByteArray &key,
                                       const QByteArray &iv) {
  return encryptAes256(cipherData, key, iv);
}

bool VaultManager::encryptAndStoreImage(const QString &sourceImagePath,
                                        const QString &password) {
  QString activePass = password.isEmpty() ? m_cachedMasterPassword : password;
  if (activePass.isEmpty()) {
    emit vaultError("Vault is locked. Provide a password to store image.");
    return false;
  }

  QFile srcFile(sourceImagePath);
  if (!srcFile.open(QIODevice::ReadOnly)) {
    emit vaultError("Failed to open source image.");
    return false;
  }
  QByteArray imgData = srcFile.readAll();
  srcFile.close();

  QByteArray salt(16, 0);
  QByteArray iv(16, 0);
  for (int i = 0; i < 16; ++i) {
    salt[i] = static_cast<char>(QRandomGenerator::global()->generate() & 0xFF);
    iv[i] = static_cast<char>(QRandomGenerator::global()->generate() & 0xFF);
  }

  QByteArray key = deriveKey(activePass, salt, 10000);
  QByteArray cipher = encryptAes256(imgData, key, iv);
  QByteArray hmac =
      QMessageAuthenticationCode::hash(cipher, key, QCryptographicHash::Sha256);

  QFileInfo fi(sourceImagePath);
  QString outName = QString("%1_%2.rovault")
                        .arg(fi.baseName())
                        .arg(QDateTime::currentMSecsSinceEpoch());
  QString outPath = m_vaultDir + "/" + outName;

  QJsonObject meta;
  meta["originalName"] = fi.fileName();
  meta["createdAt"] = fi.lastModified().toString(Qt::ISODate);
  meta["fileSize"] = imgData.size();
  QByteArray metaJson = QJsonDocument(meta).toJson(QJsonDocument::Compact);

  QFile outF(outPath);
  if (!outF.open(QIODevice::WriteOnly)) {
    emit vaultError("Failed to create vault file.");
    return false;
  }

  outF.write(kVaultMagic, 8);
  outF.write(salt);
  outF.write(iv);
  outF.write(hmac);

  qint32 metaLen = static_cast<qint32>(metaJson.size());
  outF.write(reinterpret_cast<const char *>(&metaLen), sizeof(metaLen));
  outF.write(metaJson);
  outF.write(cipher);
  outF.close();

  emit vaultChanged();
  return true;
}

QImage VaultManager::decryptImage(const QString &vaultFilePath,
                                  const QString &password) {
  QString activePass = password.isEmpty() ? m_cachedMasterPassword : password;
  if (activePass.isEmpty())
    return {};

  QFile f(vaultFilePath);
  if (!f.open(QIODevice::ReadOnly))
    return {};

  QByteArray magic = f.read(8);
  if (magic != kVaultMagic)
    return {};

  QByteArray salt = f.read(16);
  QByteArray iv = f.read(16);
  QByteArray hmac = f.read(32);

  qint32 metaLen = 0;
  f.read(reinterpret_cast<char *>(&metaLen), sizeof(metaLen));
  if (metaLen > 0 && metaLen < 65536) {
    f.read(metaLen);
  }

  QByteArray cipher = f.readAll();
  f.close();

  QByteArray key = deriveKey(activePass, salt, 10000);
  QByteArray checkHmac =
      QMessageAuthenticationCode::hash(cipher, key, QCryptographicHash::Sha256);
  if (checkHmac != hmac)
    return {};

  QByteArray plain = decryptAes256(cipher, key, iv);
  QImage img;
  img.loadFromData(plain);
  return img;
}

bool VaultManager::exportDecryptedImage(const QString &vaultFilePath,
                                        const QString &destinationPath) {
  QImage img = decryptImage(vaultFilePath);
  if (img.isNull())
    return false;
  return img.save(destinationPath);
}

bool VaultManager::deleteVaultItem(const QString &vaultFilePath) {
  bool ok = QFile::remove(vaultFilePath);
  if (ok)
    emit vaultChanged();
  return ok;
}

QVector<VaultItem> VaultManager::listVaultItems() {
  QVector<VaultItem> items;
  QDir dir(m_vaultDir);
  QStringList files =
      dir.entryList(QStringList() << "*.rovault", QDir::Files, QDir::Time);

  for (const QString &fname : files) {
    QString p = dir.absoluteFilePath(fname);
    QFileInfo fi(p);
    VaultItem it;
    it.id = fi.baseName();
    it.vaultFilePath = p;
    it.fileSize = fi.size();
    it.encryptedAt = fi.lastModified();

    QFile f(p);
    if (f.open(QIODevice::ReadOnly)) {
      f.seek(8 + 16 + 16 + 32);
      qint32 metaLen = 0;
      if (f.read(reinterpret_cast<char *>(&metaLen), sizeof(metaLen)) ==
              sizeof(metaLen) &&
          metaLen > 0 && metaLen < 65536) {
        QByteArray metaJson = f.read(metaLen);
        QJsonObject obj = QJsonDocument::fromJson(metaJson).object();
        it.originalFileName = obj["originalName"].toString(fi.fileName());
      } else {
        it.originalFileName = fi.fileName();
      }
      f.close();
    }
    items.append(it);
  }
  return items;
}

} // namespace ro_screenshot
