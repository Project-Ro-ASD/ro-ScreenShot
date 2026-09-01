#include "LibraryManager.hpp"
#include <QClipboard>
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImageReader>
#include <QStandardPaths>
#include <QUrl>

namespace ro_screenshot {

LibraryManager::LibraryManager(SettingsManager *settings, QObject *parent)
    : QAbstractListModel(parent), m_settings(settings) {
  QString cacheBase =
      QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
  if (cacheBase.isEmpty()) {
    cacheBase = QDir::homePath() + "/.cache/ro-asd/ro-screenshot";
  }
  m_cacheDir = cacheBase + "/thumbnails";
  QDir().mkpath(m_cacheDir);

  if (m_settings) {
    connect(m_settings, &SettingsManager::saveDirectoryChanged, this,
            &LibraryManager::onSaveDirectorySettingChanged);
    onSaveDirectorySettingChanged();
  }

  connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this,
          &LibraryManager::onDirectoryChanged);

  refresh();
}

int LibraryManager::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return static_cast<int>(m_filteredItems.size());
}

QVariant LibraryManager::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() < 0 ||
      index.row() >= static_cast<int>(m_filteredItems.size())) {
    return {};
  }

  const auto &item = m_filteredItems.at(index.row());
  switch (role) {
  case FileNameRole:
    return item.fileName;
  case FilePathRole:
    return item.filePath;
  case FileSizeRole:
    return item.fileSize;
  case FormattedSizeRole:
    return item.formattedSize;
  case FileDateRole:
    return item.createdAt;
  case FormattedDateRole:
    return item.formattedDate;
  case ResolutionRole:
    return item.resolution;
  case WidthRole:
    return item.width;
  case HeightRole:
    return item.height;
  case ThumbnailUrlRole:
    return item.thumbnailUrl;
  default:
    return {};
  }
}

QHash<int, QByteArray> LibraryManager::roleNames() const {
  return {{FileNameRole, "fileName"},     {FilePathRole, "filePath"},
          {FileSizeRole, "fileSize"},     {FormattedSizeRole, "formattedSize"},
          {FileDateRole, "fileDate"},     {FormattedDateRole, "formattedDate"},
          {ResolutionRole, "resolution"}, {WidthRole, "width"},
          {HeightRole, "height"},         {ThumbnailUrlRole, "thumbnailUrl"}};
}

int LibraryManager::count() const {
  return static_cast<int>(m_filteredItems.size());
}

QString LibraryManager::searchQuery() const { return m_searchQuery; }

void LibraryManager::setSearchQuery(const QString &query) {
  if (m_searchQuery != query) {
    m_searchQuery = query;
    emit searchQueryChanged();
    filterItems();
  }
}

int LibraryManager::dateFilter() const { return m_dateFilter; }

void LibraryManager::setDateFilter(int filter) {
  if (m_dateFilter != filter) {
    m_dateFilter = filter;
    emit dateFilterChanged();
    filterItems();
  }
}

bool LibraryManager::isScanning() const { return m_isScanning; }

QString LibraryManager::totalStorageSize() const {
  return formatFileSize(m_totalBytes);
}

void LibraryManager::onSaveDirectorySettingChanged() {
  if (!m_watcher.directories().isEmpty()) {
    m_watcher.removePaths(m_watcher.directories());
  }
  if (m_settings) {
    QString dir = m_settings->saveDirectory();
    if (QDir(dir).exists()) {
      m_watcher.addPath(dir);
    }
  }
  refresh();
}

void LibraryManager::onDirectoryChanged(const QString & /*path*/) { refresh(); }

void LibraryManager::refresh() {
  if (!m_settings) {
    return;
  }

  m_isScanning = true;
  emit isScanningChanged();

  QString targetDir = m_settings->saveDirectory();
  QDir dir(targetDir);
  if (!dir.exists()) {
    dir.mkpath(targetDir);
  }

  QList<ScreenshotItem> items;
  qint64 total = 0;

  QStringList nameFilters;
  nameFilters << "*.png" << "*.jpg" << "*.jpeg" << "*.webp" << "*.bmp";

  QDirIterator it(targetDir, nameFilters, QDir::Files,
                  QDirIterator::Subdirectories);
  while (it.hasNext()) {
    QString filePath = it.next();
    QFileInfo info(filePath);

    ScreenshotItem item;
    item.fileName = info.fileName();
    item.filePath = info.absoluteFilePath();
    item.fileSize = info.size();
    total += item.fileSize;
    item.formattedSize = formatFileSize(item.fileSize);
    item.createdAt = info.lastModified();
    item.formattedDate = item.createdAt.toString("yyyy-MM-dd hh:mm");

    QImageReader reader(item.filePath);
    QSize size = reader.size();
    if (size.isValid()) {
      item.width = size.width();
      item.height = size.height();
      item.resolution = QString("%1 × %2").arg(size.width()).arg(size.height());
    } else {
      item.resolution = "Unknown";
    }

    item.thumbnailUrl = getOrCreateThumbnail(item.filePath);
    items.append(item);
  }

  std::sort(items.begin(), items.end(),
            [](const ScreenshotItem &a, const ScreenshotItem &b) {
              return a.createdAt > b.createdAt;
            });

  m_allItems = std::move(items);
  m_totalBytes = total;
  emit totalStorageSizeChanged();

  m_isScanning = false;
  emit isScanningChanged();

  filterItems();
}

void LibraryManager::filterItems() {
  beginResetModel();
  m_filteredItems.clear();

  for (const auto &item : m_allItems) {
    if (passesFilter(item)) {
      m_filteredItems.append(item);
    }
  }
  endResetModel();

  emit countChanged();
}

bool LibraryManager::passesFilter(const ScreenshotItem &item) const {
  if (!m_searchQuery.isEmpty()) {
    if (!item.fileName.contains(m_searchQuery, Qt::CaseInsensitive)) {
      return false;
    }
  }

  if (m_dateFilter == 0) { // All
    return true;
  }

  QDate itemDate = item.createdAt.date();
  QDate today = QDate::currentDate();

  switch (m_dateFilter) {
  case 1: // Today
    return itemDate == today;
  case 2: // Yesterday
    return itemDate == today.addDays(-1);
  case 3: // This Week
    return itemDate >= today.addDays(-7);
  case 4: // This Month
    return itemDate.month() == today.month() && itemDate.year() == today.year();
  default:
    return true;
  }
}

bool LibraryManager::deleteItem(int row) {
  if (row < 0 || row >= static_cast<int>(m_filteredItems.size())) {
    return false;
  }

  QString path = m_filteredItems.at(row).filePath;
  return deleteItemByPath(path);
}

bool LibraryManager::deleteItemByPath(const QString &path) {
  QFile file(path);
  if (file.remove()) {
    // Delete cached thumbnail if exists
    QByteArray hash =
        QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Md5)
            .toHex();
    QString thumbPath = m_cacheDir + "/" + hash + ".png";
    QFile::remove(thumbPath);

    emit itemDeleted(path);
    refresh();
    return true;
  }
  return false;
}

bool LibraryManager::copyToClipboard(int row) {
  if (row < 0 || row >= static_cast<int>(m_filteredItems.size())) {
    return false;
  }

  QString path = m_filteredItems.at(row).filePath;
  QImage img(path);
  if (!img.isNull()) {
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setImage(img);
    return true;
  }
  return false;
}

void LibraryManager::openInFolder(int row) {
  if (row < 0 || row >= static_cast<int>(m_filteredItems.size())) {
    return;
  }

  QString path = m_filteredItems.at(row).filePath;
  QFileInfo info(path);
  QDesktopServices::openUrl(QUrl::fromLocalFile(info.absolutePath()));
}

void LibraryManager::openFile(int row) {
  if (row < 0 || row >= static_cast<int>(m_filteredItems.size())) {
    return;
  }

  QString path = m_filteredItems.at(row).filePath;
  QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

QString LibraryManager::getFilePath(int row) const {
  if (row < 0 || row >= static_cast<int>(m_filteredItems.size())) {
    return {};
  }
  return m_filteredItems.at(row).filePath;
}

QVariantMap LibraryManager::getItem(int row) const {
  QVariantMap map;
  if (row < 0 || row >= static_cast<int>(m_filteredItems.size())) {
    return map;
  }
  const auto &item = m_filteredItems.at(row);
  map["fileName"] = item.fileName;
  map["filePath"] = item.filePath;
  map["fileSize"] = item.fileSize;
  map["formattedSize"] = item.formattedSize;
  map["formattedDate"] = item.formattedDate;
  map["resolution"] = item.resolution;
  map["width"] = item.width;
  map["height"] = item.height;
  map["thumbnailUrl"] = item.thumbnailUrl;
  return map;
}

QString LibraryManager::getOrCreateThumbnail(const QString &imagePath) const {
  QByteArray hash =
      QCryptographicHash::hash(imagePath.toUtf8(), QCryptographicHash::Md5)
          .toHex();
  QString thumbPath = m_cacheDir + "/" + hash + ".png";

  if (QFile::exists(thumbPath)) {
    return QUrl::fromLocalFile(thumbPath).toString();
  }

  QImageReader reader(imagePath);
  QSize origSize = reader.size();
  if (origSize.isValid()) {
    QSize targetSize = origSize.scaled(320, 240, Qt::KeepAspectRatio);
    reader.setScaledSize(targetSize);
    QImage thumb = reader.read();
    if (!thumb.isNull()) {
      thumb.save(thumbPath, "PNG");
      return QUrl::fromLocalFile(thumbPath).toString();
    }
  }

  return QUrl::fromLocalFile(imagePath).toString();
}

QString LibraryManager::formatFileSize(qint64 bytes) const {
  double dBytes = static_cast<double>(bytes);
  if (bytes < 1024) {
    return QString("%1 B").arg(bytes);
  } else if (bytes < 1024 * 1024) {
    return QString("%1 KB").arg(QString::number(dBytes / 1024.0, 'f', 1));
  } else if (bytes < 1024 * 1024 * 1024) {
    return QString("%1 MB").arg(
        QString::number(dBytes / (1024.0 * 1024.0), 'f', 1));
  } else {
    return QString("%1 GB").arg(
        QString::number(dBytes / (1024.0 * 1024.0 * 1024.0), 'f', 2));
  }
}

} // namespace ro_screenshot
