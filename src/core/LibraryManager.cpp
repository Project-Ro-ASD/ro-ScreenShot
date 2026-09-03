#include "core/LibraryManager.hpp"
#include "core/advanced/DuplicateCleaner.hpp"
#include "core/advanced/ImageDiffEngine.hpp"
#include "core/advanced/MetadataSanitizer.hpp"
#include "core/advanced/MockupFrameGenerator.hpp"
#include "core/advanced/OcrEngine.hpp"
#include "core/advanced/PaletteExtractor.hpp"
#include "core/advanced/PdfReportGenerator.hpp"
#include "core/advanced/TableExtractor.hpp"

#include <QBuffer>
#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QMetaObject>
#include <QPainter>
#include <QSaveFile>
#include <QThread>
#include <QUrl>
#include <algorithm>

namespace ro_screenshot {

LibraryManager::LibraryManager(SettingsManager *settings, QObject *parent)
    : QAbstractListModel(parent), m_settings(settings),
      m_cache(std::make_shared<ThumbnailCache>()) {

  loadMetadata();

  if (m_settings) {
    connect(m_settings, &SettingsManager::saveDirectoryChanged, this,
            &LibraryManager::onSaveDirectorySettingChanged);
    onSaveDirectorySettingChanged();
  }

  connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this,
          &LibraryManager::onDirectoryChanged);

  refresh();
}

LibraryManager::~LibraryManager() {
  saveMetadata();
  if (m_currentCancelFlag) {
    m_currentCancelFlag->store(true, std::memory_order_relaxed);
  }
}

void LibraryManager::loadMetadata() {
  QString metaPath =
      QDir::homePath() + "/.config/ro-screenshot/library_meta.json";
  QFile f(metaPath);
  if (!f.open(QIODevice::ReadOnly))
    return;

  QJsonObject root = QJsonDocument::fromJson(f.readAll()).object();
  f.close();

  m_itemTags.clear();
  QJsonObject tagsObj = root["tags"].toObject();
  for (auto it = tagsObj.begin(); it != tagsObj.end(); ++it) {
    QJsonArray arr = it.value().toArray();
    QStringList tagList;
    for (const auto &v : arr) {
      tagList.append(v.toString());
    }
    m_itemTags.insert(it.key(), tagList);
  }

  m_favoritePaths.clear();
  QJsonArray favArr = root["favorites"].toArray();
  for (const auto &v : favArr) {
    m_favoritePaths.insert(v.toString());
  }
}

void LibraryManager::saveMetadata() {
  QString dir = QDir::homePath() + "/.config/ro-screenshot";
  QDir().mkpath(dir);
  QString metaPath = dir + "/library_meta.json";

  QJsonObject root;
  QJsonObject tagsObj;
  for (auto it = m_itemTags.begin(); it != m_itemTags.end(); ++it) {
    QJsonArray arr;
    for (const QString &t : it.value()) {
      arr.append(t);
    }
    tagsObj[it.key()] = arr;
  }
  root["tags"] = tagsObj;

  QJsonArray favArr;
  for (const QString &p : m_favoritePaths) {
    favArr.append(p);
  }
  root["favorites"] = favArr;

  QFile f(metaPath);
  if (f.open(QIODevice::WriteOnly)) {
    f.write(QJsonDocument(root).toJson());
    f.close();
  }
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
  case IsSelectedRole:
    return m_selectedPaths.contains(item.filePath);
  case FormatRole: {
    QString suffix = QFileInfo(item.filePath).suffix().toUpper();
    return suffix.isEmpty() ? "PNG" : suffix;
  }
  case TagsRole:
    return m_itemTags.value(item.filePath);
  case IsFavoriteRole:
    return m_favoritePaths.contains(item.filePath);
  default:
    return {};
  }
}

QHash<int, QByteArray> LibraryManager::roleNames() const {
  return {{FileNameRole, "fileName"},
          {FilePathRole, "filePath"},
          {FileSizeRole, "fileSize"},
          {FormattedSizeRole, "formattedSize"},
          {FileDateRole, "fileDate"},
          {FormattedDateRole, "formattedDate"},
          {ResolutionRole, "resolution"},
          {WidthRole, "width"},
          {HeightRole, "height"},
          {ThumbnailUrlRole, "thumbnailUrl"},
          {IsSelectedRole, "isSelected"},
          {FormatRole, "format"},
          {TagsRole, "tags"},
          {IsFavoriteRole, "isFavorite"}};
}

int LibraryManager::count() const {
  return static_cast<int>(m_filteredItems.size());
}

QString LibraryManager::searchQuery() const { return m_searchQuery; }

void LibraryManager::setSearchQuery(const QString &query) {
  if (m_searchQuery != query) {
    m_searchQuery = query;
    emit searchQueryChanged();
    filterAndSortItems();
  }
}

int LibraryManager::dateFilter() const { return m_dateFilter; }

void LibraryManager::setDateFilter(int filter) {
  if (m_dateFilter != filter) {
    m_dateFilter = filter;
    emit dateFilterChanged();
    filterAndSortItems();
  }
}

int LibraryManager::formatFilter() const { return m_formatFilter; }

void LibraryManager::setFormatFilter(int filter) {
  if (m_formatFilter != filter) {
    m_formatFilter = filter;
    emit formatFilterChanged();
    filterAndSortItems();
  }
}

QString LibraryManager::tagFilter() const { return m_tagFilter; }

void LibraryManager::setTagFilter(const QString &tag) {
  if (m_tagFilter != tag) {
    m_tagFilter = tag;
    emit tagFilterChanged();
    filterAndSortItems();
  }
}

bool LibraryManager::favoritesOnly() const { return m_favoritesOnly; }

void LibraryManager::setFavoritesOnly(bool favOnly) {
  if (m_favoritesOnly != favOnly) {
    m_favoritesOnly = favOnly;
    emit favoritesOnlyChanged();
    filterAndSortItems();
  }
}

QStringList LibraryManager::allTags() const {
  QSet<QString> uniqueTags;
  for (const auto &tags : m_itemTags) {
    for (const auto &t : tags) {
      if (!t.trimmed().isEmpty()) {
        uniqueTags.insert(t.trimmed());
      }
    }
  }
  QStringList list = uniqueTags.values();
  list.sort();
  return list;
}

int LibraryManager::sortOrder() const { return m_sortOrder; }

void LibraryManager::setSortOrder(int order) {
  if (m_sortOrder != order) {
    m_sortOrder = order;
    emit sortOrderChanged();
    filterAndSortItems();
  }
}

bool LibraryManager::isScanning() const { return m_isScanning; }
QString LibraryManager::scanState() const { return m_scanState; }
int LibraryManager::scanProgress() const { return m_scanProgress; }

QString LibraryManager::totalStorageSize() const {
  return formatFileSize(m_totalBytes);
}

int LibraryManager::selectedCount() const {
  return static_cast<int>(m_selectedPaths.size());
}

bool LibraryManager::canUndoTrash() const { return m_trashManager.canUndo(); }

QString LibraryManager::lastTrashedFileName() const {
  return m_lastTrashedFileName;
}

QString LibraryManager::errorMessage() const { return m_errorMessage; }

void LibraryManager::onSaveDirectorySettingChanged() {
  if (!m_settings) {
    return;
  }
  QString dir = m_settings->saveDirectory();
  if (QDir(dir).exists()) {
    updateWatchers({dir});
  }
  refresh();
}

void LibraryManager::onDirectoryChanged(const QString & /*path*/) { refresh(); }

void LibraryManager::updateWatchers(const QStringList &directories) {
  if (!m_watcher.directories().isEmpty()) {
    m_watcher.removePaths(m_watcher.directories());
  }
  QStringList validDirs;
  for (const auto &d : directories) {
    if (QDir(d).exists()) {
      validDirs.append(d);
    }
  }
  if (!validDirs.isEmpty()) {
    m_watcher.addPaths(validDirs);
  }
}

void LibraryManager::refresh() {
  if (!m_settings) {
    return;
  }

  QString targetDir = m_settings->saveDirectory();
  QDir dir(targetDir);
  if (!dir.exists()) {
    dir.mkpath(targetDir);
  }

  // Cancel any running scan
  if (m_currentCancelFlag) {
    m_currentCancelFlag->store(true, std::memory_order_relaxed);
  }

  m_currentGeneration++;
  m_currentCancelFlag = std::make_shared<std::atomic<bool>>(false);

  m_isScanning = true;
  m_scanState = "scanning";
  m_scanProgress = 0;
  emit isScanningChanged();
  emit scanStateChanged();
  emit scanProgressChanged();

  auto *worker = new ScanWorker(
      m_currentGeneration, targetDir, m_cache, m_currentCancelFlag, this,
      [this](int count) { onScanProgress(count); },
      [this](const ScanResult &result) { onScanFinished(result); });

  QThreadPool::globalInstance()->start(worker);
}

bool LibraryManager::waitForScan(int timeoutMs) {
  if (!m_isScanning) {
    return true;
  }
  QElapsedTimer timer;
  timer.start();
  while (m_isScanning && timer.elapsed() < timeoutMs) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    QThread::msleep(5);
  }
  return !m_isScanning;
}

void LibraryManager::onScanProgress(int count) {
  m_scanProgress = count;
  emit scanProgressChanged();
}

void LibraryManager::onScanFinished(const ro_screenshot::ScanResult &result) {
  if (result.generation != m_currentGeneration) {
    return;
  }

  m_isScanning = false;
  if (!result.success) {
    m_scanState = "error";
    m_errorMessage = result.errorMessage;
    emit scanStateChanged();
    emit errorMessageChanged();
    emit isScanningChanged();
    return;
  }

  m_scanState = "idle";
  m_errorMessage.clear();
  emit scanStateChanged();
  emit errorMessageChanged();
  emit isScanningChanged();

  m_allItems = result.items;
  m_totalBytes = result.totalBytes;
  emit totalStorageSizeChanged();

  updateWatchers(result.discoveredDirectories);
  filterAndSortItems();
}

void LibraryManager::filterAndSortItems() {
  beginResetModel();
  m_filteredItems.clear();

  for (const auto &item : m_allItems) {
    if (passesFilter(item)) {
      m_filteredItems.append(item);
    }
  }

  switch (m_sortOrder) {
  case NewestFirst:
    std::sort(m_filteredItems.begin(), m_filteredItems.end(),
              [](const ScreenshotItem &a, const ScreenshotItem &b) {
                return a.createdAt > b.createdAt;
              });
    break;
  case OldestFirst:
    std::sort(m_filteredItems.begin(), m_filteredItems.end(),
              [](const ScreenshotItem &a, const ScreenshotItem &b) {
                return a.createdAt < b.createdAt;
              });
    break;
  case NameAsc:
    std::sort(m_filteredItems.begin(), m_filteredItems.end(),
              [](const ScreenshotItem &a, const ScreenshotItem &b) {
                return a.fileName.localeAwareCompare(b.fileName) < 0;
              });
    break;
  case NameDesc:
    std::sort(m_filteredItems.begin(), m_filteredItems.end(),
              [](const ScreenshotItem &a, const ScreenshotItem &b) {
                return a.fileName.localeAwareCompare(b.fileName) > 0;
              });
    break;
  case SizeDesc:
    std::sort(m_filteredItems.begin(), m_filteredItems.end(),
              [](const ScreenshotItem &a, const ScreenshotItem &b) {
                return a.fileSize > b.fileSize;
              });
    break;
  case SizeAsc:
    std::sort(m_filteredItems.begin(), m_filteredItems.end(),
              [](const ScreenshotItem &a, const ScreenshotItem &b) {
                return a.fileSize < b.fileSize;
              });
    break;
  case ResolutionDesc:
    std::sort(m_filteredItems.begin(), m_filteredItems.end(),
              [](const ScreenshotItem &a, const ScreenshotItem &b) {
                return (a.width * a.height) > (b.width * b.height);
              });
    break;
  default:
    break;
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

  // Tag filter
  if (!m_tagFilter.isEmpty()) {
    QStringList tags = m_itemTags.value(item.filePath);
    if (!tags.contains(m_tagFilter, Qt::CaseInsensitive)) {
      return false;
    }
  }

  // Favorites filter
  if (m_favoritesOnly) {
    if (!m_favoritePaths.contains(item.filePath)) {
      return false;
    }
  }

  // Format filter
  if (m_formatFilter > 0) {
    QString ext = QFileInfo(item.filePath).suffix().toLower();
    if (m_formatFilter == 1 && ext != "png") {
      return false;
    }
    if (m_formatFilter == 2 && ext != "jpg" && ext != "jpeg") {
      return false;
    }
    if (m_formatFilter == 3 && ext != "webp") {
      return false;
    }
  }

  // Date filter
  if (m_dateFilter == 0) {
    return true;
  }

  QDate itemDate = item.createdAt.date();
  QDate today = QDate::currentDate();

  switch (m_dateFilter) {
  case 1: // Today
    return itemDate == today;
  case 2: // Yesterday
    return itemDate == today.addDays(-1);
  case 3: { // This Week
    int dayOfWeek = today.dayOfWeek();
    QDate startOfWeek = today.addDays(-(dayOfWeek - 1));
    QDate endOfWeek = startOfWeek.addDays(6);
    return itemDate >= startOfWeek && itemDate <= endOfWeek;
  }
  case 4: // Last 7 Days
    return itemDate >= today.addDays(-6) && itemDate <= today;
  case 5: // This Month
    return itemDate.month() == today.month() && itemDate.year() == today.year();
  default:
    return true;
  }
}

void LibraryManager::addTag(int row, const QString &tag) {
  if (row < 0 || row >= m_filteredItems.size() || tag.trimmed().isEmpty())
    return;
  QString path = m_filteredItems[row].filePath;
  QString cleanTag = tag.trimmed();
  QStringList tags = m_itemTags.value(path);
  if (!tags.contains(cleanTag)) {
    tags.append(cleanTag);
    m_itemTags[path] = tags;
    saveMetadata();
    emit tagsChanged();
    emit dataChanged(index(row, 0), index(row, 0), {TagsRole});
  }
}

void LibraryManager::removeTag(int row, const QString &tag) {
  if (row < 0 || row >= m_filteredItems.size())
    return;
  QString path = m_filteredItems[row].filePath;
  QStringList tags = m_itemTags.value(path);
  if (tags.removeAll(tag) > 0) {
    m_itemTags[path] = tags;
    saveMetadata();
    emit tagsChanged();
    emit dataChanged(index(row, 0), index(row, 0), {TagsRole});
  }
}

QStringList LibraryManager::getTags(int row) const {
  if (row < 0 || row >= m_filteredItems.size())
    return {};
  return m_itemTags.value(m_filteredItems[row].filePath);
}

void LibraryManager::toggleFavorite(int row) {
  if (row < 0 || row >= m_filteredItems.size())
    return;
  QString path = m_filteredItems[row].filePath;
  if (m_favoritePaths.contains(path)) {
    m_favoritePaths.remove(path);
  } else {
    m_favoritePaths.insert(path);
  }
  saveMetadata();
  emit dataChanged(index(row, 0), index(row, 0), {IsFavoriteRole});
}

bool LibraryManager::isFavorite(int row) const {
  if (row < 0 || row >= m_filteredItems.size())
    return false;
  return m_favoritePaths.contains(m_filteredItems[row].filePath);
}

bool LibraryManager::deleteItem(int row) {
  if (row < 0 || row >= static_cast<int>(m_filteredItems.size())) {
    return false;
  }
  QString path = m_filteredItems.at(row).filePath;
  return deleteItemByPath(path);
}

bool LibraryManager::deleteItemByPath(const QString &path) {
  if (!m_settings) {
    return false;
  }

  QString root = m_settings->saveDirectory();
  QString err;
  if (m_trashManager.trashFile(path, root, &err)) {
    m_lastTrashedFileName = QFileInfo(path).fileName();
    emit lastTrashedChanged();
    emit canUndoTrashChanged();
    emit itemTrashed(path);
    emit itemDeleted(path);

    if (m_selectedPaths.remove(path)) {
      emit selectionChanged();
    }

    refresh();
    return true;
  }

  m_errorMessage = err;
  emit errorMessageChanged();
  return false;
}

bool LibraryManager::undoLastTrash() {
  QString restoredPath;
  QString err;
  if (m_trashManager.undoLastTrash(&restoredPath, &err)) {
    emit canUndoTrashChanged();
    emit itemRestored(restoredPath);
    emit operationCompleted("Dosya geri yüklendi: " +
                            QFileInfo(restoredPath).fileName());
    refresh();
    return true;
  }

  m_errorMessage = err;
  emit errorMessageChanged();
  return false;
}

bool LibraryManager::permanentDeleteItem(int row) {
  if (row < 0 || row >= static_cast<int>(m_filteredItems.size()) ||
      !m_settings) {
    return false;
  }

  QString path = m_filteredItems.at(row).filePath;
  QString root = m_settings->saveDirectory();
  QString err;

  if (m_trashManager.permanentDelete(path, root, &err)) {
    emit itemDeleted(path);
    if (m_selectedPaths.remove(path)) {
      emit selectionChanged();
    }
    refresh();
    return true;
  }

  m_errorMessage = err;
  emit errorMessageChanged();
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
    emit operationCompleted("Görsel panoya kopyalandı.");
    return true;
  }
  return false;
}

void LibraryManager::openInFolder(int row) {
  if (row < 0 || row >= static_cast<int>(m_filteredItems.size())) {
    return;
  }

  QString path = m_filteredItems.at(row).filePath;
  m_trashManager.showInFolder(path);
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
  map["isSelected"] = m_selectedPaths.contains(item.filePath);
  map["tags"] = m_itemTags.value(item.filePath);
  map["isFavorite"] = m_favoritePaths.contains(item.filePath);
  return map;
}

void LibraryManager::toggleSelection(int row) {
  if (row < 0 || row >= static_cast<int>(m_filteredItems.size())) {
    return;
  }
  QString path = m_filteredItems.at(row).filePath;
  if (m_selectedPaths.contains(path)) {
    m_selectedPaths.remove(path);
  } else {
    m_selectedPaths.insert(path);
  }
  emit dataChanged(index(row, 0), index(row, 0), {IsSelectedRole});
  emit selectionChanged();
}

void LibraryManager::selectAll() {
  m_selectedPaths.clear();
  for (const auto &item : m_filteredItems) {
    m_selectedPaths.insert(item.filePath);
  }
  if (!m_filteredItems.isEmpty()) {
    emit dataChanged(index(0, 0),
                     index(static_cast<int>(m_filteredItems.size()) - 1, 0),
                     {IsSelectedRole});
  }
  emit selectionChanged();
}

void LibraryManager::clearSelection() {
  m_selectedPaths.clear();
  if (!m_filteredItems.isEmpty()) {
    emit dataChanged(index(0, 0),
                     index(static_cast<int>(m_filteredItems.size()) - 1, 0),
                     {IsSelectedRole});
  }
  emit selectionChanged();
}

bool LibraryManager::isSelected(int row) const {
  if (row < 0 || row >= static_cast<int>(m_filteredItems.size())) {
    return false;
  }
  return m_selectedPaths.contains(m_filteredItems.at(row).filePath);
}

QList<int> LibraryManager::selectedRows() const {
  QList<int> rows;
  for (int i = 0; i < m_filteredItems.size(); ++i) {
    if (m_selectedPaths.contains(m_filteredItems.at(i).filePath)) {
      rows.append(i);
    }
  }
  return rows;
}

bool LibraryManager::trashSelected() {
  if (m_selectedPaths.isEmpty() || !m_settings) {
    return false;
  }

  QString root = m_settings->saveDirectory();
  int countSuccess = 0;
  for (const auto &path : m_selectedPaths) {
    if (m_trashManager.trashFile(path, root)) {
      countSuccess++;
    }
  }

  m_selectedPaths.clear();
  emit selectionChanged();
  emit canUndoTrashChanged();
  refresh();
  return countSuccess > 0;
}

bool LibraryManager::permanentDeleteSelected() {
  if (m_selectedPaths.isEmpty() || !m_settings) {
    return false;
  }

  QString root = m_settings->saveDirectory();
  int countSuccess = 0;
  for (const auto &path : m_selectedPaths) {
    if (m_trashManager.permanentDelete(path, root)) {
      countSuccess++;
    }
  }

  m_selectedPaths.clear();
  emit selectionChanged();
  refresh();
  return countSuccess > 0;
}

bool LibraryManager::copySelectedToClipboard() {
  if (m_selectedPaths.isEmpty()) {
    return false;
  }

  QString firstPath = *m_selectedPaths.begin();
  QImage img(firstPath);
  if (!img.isNull()) {
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setImage(img);
    emit operationCompleted("Seçili görsel panoya kopyalandı.");
    return true;
  }
  return false;
}

QVariantMap LibraryManager::compareSelectedImages() {
  QVariantMap res;
  if (m_selectedPaths.size() < 2) {
    res["success"] = false;
    res["error"] = "Karşılaştırma için en az 2 görsel seçmelisiniz.";
    return res;
  }

  auto it = m_selectedPaths.begin();
  QString pathA = *it++;
  QString pathB = *it;

  QImage imgA(pathA);
  QImage imgB(pathB);

  DiffResult diff = ImageDiffEngine::compare(imgA, imgB);
  res["success"] = true;
  res["pathA"] = pathA;
  res["pathB"] = pathB;
  res["differentPixels"] = diff.differentPixels;
  res["similarityPercent"] = diff.similarityPercent;
  res["differencePercent"] = diff.differencePercent;
  res["dimensionsMatch"] = diff.dimensionsMatch;

  return res;
}

QVariantMap LibraryManager::batchConvertSelected(const QString &targetFormat,
                                                 int quality,
                                                 double scaleFactor,
                                                 const QString &targetDir,
                                                 const QString &renamePattern) {
  QVariantMap res;
  if (m_selectedPaths.isEmpty()) {
    res["success"] = false;
    res["error"] = "Hiçbir görsel seçilmedi.";
    return res;
  }

  int successCount = 0;
  QString outDir =
      targetDir.isEmpty() ? m_settings->saveDirectory() : targetDir;
  QDir().mkpath(outDir);

  int idx = 1;
  for (const QString &srcPath : m_selectedPaths) {
    QImage img(srcPath);
    if (img.isNull())
      continue;

    if (scaleFactor > 0.05 && std::abs(scaleFactor - 1.0) > 0.01) {
      int newW = std::max(1, static_cast<int>(img.width() * scaleFactor));
      int newH = std::max(1, static_cast<int>(img.height() * scaleFactor));
      img =
          img.scaled(newW, newH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    QFileInfo fi(srcPath);
    QString baseName = renamePattern.isEmpty() ? fi.completeBaseName()
                                               : renamePattern.arg(idx++);
    QString ext = targetFormat.toLower();
    if (ext.isEmpty())
      ext = fi.suffix().toLower();

    QString destPath = QString("%1/%2.%3").arg(outDir, baseName, ext);
    if (img.save(destPath, ext.toLatin1().constData(), quality)) {
      successCount++;
    }
  }

  res["success"] = (successCount > 0);
  res["convertedCount"] = successCount;
  emit operationCompleted(
      QString("%1 görsel başarıyla dönüştürüldü.").arg(successCount));
  refresh();
  return res;
}

QVariantMap LibraryManager::findDuplicatesInLibrary(int tolerance) {
  QStringList allPaths;
  for (const auto &it : m_allItems) {
    allPaths.append(it.filePath);
  }

  QVector<DuplicateGroup> groups =
      DuplicateCleaner::findDuplicates(allPaths, tolerance);
  QVariantMap res;
  res["groupCount"] = groups.size();

  qint64 totalReclaim = 0;
  QVariantList groupList;
  for (const auto &g : groups) {
    QVariantMap gm;
    gm["primary"] = g.primaryFilePath;
    gm["duplicates"] = g.duplicateFilePaths;
    gm["reclaimableBytes"] = g.totalReclaimableBytes;
    gm["reclaimableFormatted"] = formatFileSize(g.totalReclaimableBytes);
    totalReclaim += g.totalReclaimableBytes;
    groupList.append(gm);
  }

  res["totalReclaimable"] = totalReclaim;
  res["totalReclaimableFormatted"] = formatFileSize(totalReclaim);
  res["groups"] = groupList;

  emit duplicateScanFinished(groups.size(), totalReclaim);
  return res;
}

bool LibraryManager::generatePdfReportFromSelected(const QString &outputPath,
                                                   const QString &title,
                                                   const QString &notes) {
  if (m_selectedPaths.isEmpty())
    return false;

  QVector<PdfReportEntry> entries;
  for (const QString &p : m_selectedPaths) {
    PdfReportEntry e;
    e.imagePath = p;
    e.title = QFileInfo(p).fileName();
    e.tags = m_itemTags.value(p).join(", ");
    e.timestamp = QFileInfo(p).lastModified();
    entries.append(e);
  }

  PdfReportOptions opt;
  if (!title.isEmpty())
    opt.title = title;
  if (!notes.isEmpty())
    opt.notes = notes;

  bool ok = PdfReportGenerator::generateReport(outputPath, entries, opt);
  if (ok) {
    emit operationCompleted("PDF Raporu başarıyla oluşturuldu.");
  }
  return ok;
}

QVariantList LibraryManager::extractPaletteFromItem(int row, int maxColors) {
  if (row < 0 || row >= m_filteredItems.size())
    return {};
  QImage img(m_filteredItems[row].filePath);
  if (img.isNull())
    return {};

  QVector<PaletteColor> colors = PaletteExtractor::extract(img, maxColors);
  QVariantList list;
  for (const auto &c : colors) {
    QVariantMap cm;
    cm["hex"] = c.hex;
    cm["rgb"] = c.rgb;
    cm["hsl"] = c.hsl;
    cm["tailwindClass"] = c.tailwindClass;
    cm["tailwindName"] = c.tailwindName;
    cm["dominancePercent"] = c.dominancePercent;
    list.append(cm);
  }
  return list;
}

QString LibraryManager::extractTableFromItem(int row, const QString &format) {
  if (row < 0 || row >= m_filteredItems.size())
    return {};
  QImage img(m_filteredItems[row].filePath);
  if (img.isNull())
    return {};

  OcrEngine ocr;
  OcrResult ocrRes = ocr.recognize(img);
  ExtractedTable tbl = TableExtractor::extractFromOcr(ocrRes.blocks);
  if (!tbl.isValid) {
    tbl = TableExtractor::extractFromText(ocrRes.fullText);
  }

  if (format == "csv") {
    return TableExtractor::toCsv(tbl);
  } else if (format == "json") {
    return TableExtractor::toJson(tbl);
  }
  return TableExtractor::toMarkdown(tbl);
}

QString LibraryManager::exportWithMockupFrame(int row, int presetIndex,
                                              int padding,
                                              const QString &outputPath) {
  if (row < 0 || row >= m_filteredItems.size())
    return {};
  QString src = m_filteredItems[row].filePath;
  QImage img(src);
  if (img.isNull())
    return {};

  MockupOptions opt;
  opt.preset = static_cast<MockupPreset>(std::clamp(presetIndex, 0, 6));
  opt.padding = padding;

  QImage framed = MockupFrameGenerator::generate(img, opt);
  QFileInfo fi(src);
  QString dest = outputPath.isEmpty()
                     ? fi.dir().filePath(fi.baseName() + "_mockup.png")
                     : outputPath;

  if (framed.save(dest, "PNG")) {
    emit operationCompleted("Mockup çerçeveli görsel kaydedildi.");
    refresh();
    return dest;
  }
  return {};
}

QVariantMap LibraryManager::renameItem(int row, const QString &newName) {
  QVariantMap result;
  if (row < 0 || row >= static_cast<int>(m_filteredItems.size()) ||
      !m_settings) {
    result["success"] = false;
    result["error"] = "Geçersiz öğe seçimi.";
    return result;
  }

  QString oldPath = m_filteredItems.at(row).filePath;
  QString root = m_settings->saveDirectory();
  result = m_trashManager.renameFile(oldPath, newName, root);

  if (result["success"].toBool()) {
    QString newPath = result["newPath"].toString();
    if (m_selectedPaths.remove(oldPath)) {
      m_selectedPaths.insert(newPath);
    }
    if (m_itemTags.contains(oldPath)) {
      m_itemTags[newPath] = m_itemTags.take(oldPath);
    }
    if (m_favoritePaths.remove(oldPath)) {
      m_favoritePaths.insert(newPath);
    }
    saveMetadata();
    emit operationCompleted("Dosya yeniden adlandırıldı.");
    refresh();
  } else {
    m_errorMessage = result["error"].toString();
    emit errorMessageChanged();
  }

  return result;
}

QVariantMap LibraryManager::exportAnnotatedImage(
    const QString &sourcePath, const QString &overlayDataUrl, qreal viewportX,
    qreal viewportY, qreal viewportWidth, qreal viewportHeight) {
  QVariantMap result;
  result[QStringLiteral("success")] = false;

  if (!m_settings ||
      !m_trashManager.isPathSafe(sourcePath, m_settings->saveDirectory())) {
    result[QStringLiteral("error")] =
        QStringLiteral("Güvenlik hatası: Dosya kitaplık sınırları dışında.");
    return result;
  }
  if (viewportWidth <= 0.0 || viewportHeight <= 0.0) {
    result[QStringLiteral("error")] =
        QStringLiteral("Düzenleyici görüntü alanı geçersiz.");
    return result;
  }

  const qsizetype comma = overlayDataUrl.indexOf(u',');
  if (comma < 0) {
    result[QStringLiteral("error")] =
        QStringLiteral("Anotasyon verisi okunamadı.");
    return result;
  }
  const QImage source(sourcePath);
  const QImage overlay = QImage::fromData(
      QByteArray::fromBase64(overlayDataUrl.mid(comma + 1).toLatin1()), "PNG");
  if (source.isNull() || overlay.isNull()) {
    result[QStringLiteral("error")] =
        QStringLiteral("Görüntü veya anotasyon verisi okunamadı.");
    return result;
  }

  const QRect viewport =
      QRectF(viewportX, viewportY, viewportWidth, viewportHeight)
          .toAlignedRect()
          .intersected(overlay.rect());
  if (viewport.isEmpty()) {
    result[QStringLiteral("error")] =
        QStringLiteral("Anotasyon alanı görüntü sınırları dışında.");
    return result;
  }

  QImage composited =
      source.convertToFormat(QImage::Format_ARGB32_Premultiplied);
  const QImage annotations = overlay.copy(viewport).scaled(
      composited.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  QPainter painter(&composited);
  painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
  painter.drawImage(QPoint(), annotations);
  painter.end();

  const QFileInfo sourceInfo(sourcePath);
  const QString destination = sourceInfo.dir().filePath(
      QStringLiteral("%1_edited_%2.png")
          .arg(sourceInfo.completeBaseName(),
               QDateTime::currentDateTime().toString(
                   QStringLiteral("yyyyMMdd_HHmmss_zzz"))));
  QBuffer buffer;
  buffer.open(QIODevice::WriteOnly);
  if (!composited.save(&buffer, "PNG")) {
    result[QStringLiteral("error")] =
        QStringLiteral("Düzenlenmiş görüntü kodlanamadı.");
    return result;
  }
  QSaveFile output(destination);
  if (!output.open(QIODevice::WriteOnly) ||
      output.write(buffer.data()) != buffer.data().size() || !output.commit()) {
    result[QStringLiteral("error")] =
        QStringLiteral("Düzenlenmiş görüntü güvenli biçimde kaydedilemedi.");
    return result;
  }

  result[QStringLiteral("success")] = true;
  result[QStringLiteral("path")] = destination;
  emit operationCompleted(QStringLiteral("Düzenlenmiş görsel kaydedildi."));
  refresh();
  return result;
}

bool LibraryManager::clearThumbnailCache() {
  if (m_cache) {
    bool ok = m_cache->clearCache();
    emit thumbnailCacheCleared(ok);
    return ok;
  }
  return false;
}

bool LibraryManager::isPathInLibrary(const QString &path) const {
  if (!m_settings) {
    return false;
  }
  return m_trashManager.isPathSafe(path, m_settings->saveDirectory());
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
