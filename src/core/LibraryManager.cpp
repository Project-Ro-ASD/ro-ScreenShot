#include "core/LibraryManager.hpp"
#include <QBuffer>
#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QGuiApplication>
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
  if (m_currentCancelFlag) {
    m_currentCancelFlag->store(true, std::memory_order_relaxed);
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
  default:
    return {};
  }
}

QHash<int, QByteArray> LibraryManager::roleNames() const {
  return {{FileNameRole, "fileName"},     {FilePathRole, "filePath"},
          {FileSizeRole, "fileSize"},     {FormattedSizeRole, "formattedSize"},
          {FileDateRole, "fileDate"},     {FormattedDateRole, "formattedDate"},
          {ResolutionRole, "resolution"}, {WidthRole, "width"},
          {HeightRole, "height"},         {ThumbnailUrlRole, "thumbnailUrl"},
          {IsSelectedRole, "isSelected"}, {FormatRole, "format"}};
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
  // Reject stale generations
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

  // Apply sorting
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
  case 3: { // This Week (Locale-aware calendar week Monday-Sunday)
    int dayOfWeek = today.dayOfWeek(); // 1 = Monday, 7 = Sunday
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

    // Remove from selection if selected
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

  // Copy first selected image to clipboard
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
    if (m_selectedPaths.remove(oldPath)) {
      m_selectedPaths.insert(result["newPath"].toString());
    }
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
