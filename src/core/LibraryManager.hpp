#pragma once

#include "core/SettingsManager.hpp"
#include "core/Types.hpp"
#include "core/library/ScanWorker.hpp"
#include "core/library/ThumbnailCache.hpp"
#include "core/library/TrashManager.hpp"
#include <QAbstractListModel>
#include <QDateTime>
#include <QFileSystemWatcher>
#include <QList>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QString>
#include <QThreadPool>
#include <atomic>
#include <memory>

namespace ro_screenshot {

class LibraryManager : public QAbstractListModel {
  Q_OBJECT

  Q_PROPERTY(int count READ count NOTIFY countChanged)
  Q_PROPERTY(QString searchQuery READ searchQuery WRITE setSearchQuery NOTIFY
                 searchQueryChanged)
  Q_PROPERTY(int dateFilter READ dateFilter WRITE setDateFilter NOTIFY
                 dateFilterChanged)
  Q_PROPERTY(int formatFilter READ formatFilter WRITE setFormatFilter NOTIFY
                 formatFilterChanged)
  Q_PROPERTY(QString tagFilter READ tagFilter WRITE setTagFilter NOTIFY
                 tagFilterChanged)
  Q_PROPERTY(bool favoritesOnly READ favoritesOnly WRITE setFavoritesOnly NOTIFY
                 favoritesOnlyChanged)
  Q_PROPERTY(QStringList allTags READ allTags NOTIFY tagsChanged)
  Q_PROPERTY(
      int sortOrder READ sortOrder WRITE setSortOrder NOTIFY sortOrderChanged)
  Q_PROPERTY(bool isScanning READ isScanning NOTIFY isScanningChanged)
  Q_PROPERTY(QString scanState READ scanState NOTIFY scanStateChanged)
  Q_PROPERTY(int scanProgress READ scanProgress NOTIFY scanProgressChanged)
  Q_PROPERTY(QString totalStorageSize READ totalStorageSize NOTIFY
                 totalStorageSizeChanged)
  Q_PROPERTY(int selectedCount READ selectedCount NOTIFY selectionChanged)
  Q_PROPERTY(bool canUndoTrash READ canUndoTrash NOTIFY canUndoTrashChanged)
  Q_PROPERTY(QString lastTrashedFileName READ lastTrashedFileName NOTIFY
                 lastTrashedChanged)
  Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

public:
  enum Roles {
    FileNameRole = Qt::UserRole + 1,
    FilePathRole,
    FileSizeRole,
    FormattedSizeRole,
    FileDateRole,
    FormattedDateRole,
    ResolutionRole,
    WidthRole,
    HeightRole,
    ThumbnailUrlRole,
    IsSelectedRole,
    FormatRole,
    TagsRole,
    IsFavoriteRole
  };
  Q_ENUM(Roles)

  enum SortOrder {
    NewestFirst = 0,
    OldestFirst,
    NameAsc,
    NameDesc,
    SizeDesc,
    SizeAsc,
    ResolutionDesc
  };
  Q_ENUM(SortOrder)

  explicit LibraryManager(SettingsManager *settings, QObject *parent = nullptr);
  ~LibraryManager() override;

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;

  int count() const;
  QString searchQuery() const;
  void setSearchQuery(const QString &query);

  int dateFilter() const;
  void setDateFilter(int filter);

  int formatFilter() const;
  void setFormatFilter(int filter);

  QString tagFilter() const;
  void setTagFilter(const QString &tag);

  bool favoritesOnly() const;
  void setFavoritesOnly(bool favOnly);

  QStringList allTags() const;

  int sortOrder() const;
  void setSortOrder(int order);

  bool isScanning() const;
  QString scanState() const;
  int scanProgress() const;
  QString totalStorageSize() const;

  int selectedCount() const;
  bool canUndoTrash() const;
  QString lastTrashedFileName() const;
  QString errorMessage() const;

  Q_INVOKABLE void refresh();
  Q_INVOKABLE bool waitForScan(int timeoutMs = 5000);
  Q_INVOKABLE bool deleteItem(int row);
  Q_INVOKABLE bool deleteItemByPath(const QString &path);
  Q_INVOKABLE bool undoLastTrash();
  Q_INVOKABLE bool permanentDeleteItem(int row);
  Q_INVOKABLE bool copyToClipboard(int row);
  Q_INVOKABLE void openInFolder(int row);
  Q_INVOKABLE void openFile(int row);
  Q_INVOKABLE QString getFilePath(int row) const;
  Q_INVOKABLE QVariantMap getItem(int row) const;

  // Tagging and Favorites
  Q_INVOKABLE void addTag(int row, const QString &tag);
  Q_INVOKABLE void removeTag(int row, const QString &tag);
  Q_INVOKABLE QStringList getTags(int row) const;
  Q_INVOKABLE void toggleFavorite(int row);
  Q_INVOKABLE bool isFavorite(int row) const;

  // Multi-selection & Batch Operations
  Q_INVOKABLE void toggleSelection(int row);
  Q_INVOKABLE void selectAll();
  Q_INVOKABLE void clearSelection();
  Q_INVOKABLE bool isSelected(int row) const;
  Q_INVOKABLE QList<int> selectedRows() const;
  Q_INVOKABLE bool trashSelected();
  Q_INVOKABLE bool permanentDeleteSelected();
  Q_INVOKABLE bool copySelectedToClipboard();

  // Advanced Innovated Operations
  Q_INVOKABLE QVariantMap compareSelectedImages();
  Q_INVOKABLE QVariantMap batchConvertSelected(
      const QString &targetFormat, int quality = 90, double scaleFactor = 1.0,
      const QString &targetDir = QString(),
      const QString &renamePattern = QString());
  Q_INVOKABLE QVariantMap findDuplicatesInLibrary(int tolerance = 6);
  Q_INVOKABLE bool
  generatePdfReportFromSelected(const QString &outputPath,
                                const QString &title = QString(),
                                const QString &notes = QString());
  Q_INVOKABLE QVariantList extractPaletteFromItem(int row, int maxColors = 6);
  Q_INVOKABLE QString extractTableFromItem(int row,
                                           const QString &format = "markdown");
  Q_INVOKABLE QString
  exportWithMockupFrame(int row, int presetIndex = 0, int padding = 48,
                        const QString &outputPath = QString());

  // Rename & Cache
  Q_INVOKABLE QVariantMap renameItem(int row, const QString &newName);
  Q_INVOKABLE QVariantMap exportAnnotatedImage(const QString &sourcePath,
                                               const QString &overlayDataUrl,
                                               qreal viewportX, qreal viewportY,
                                               qreal viewportWidth,
                                               qreal viewportHeight);
  Q_INVOKABLE bool clearThumbnailCache();
  Q_INVOKABLE bool isPathInLibrary(const QString &path) const;

signals:
  void countChanged();
  void searchQueryChanged();
  void dateFilterChanged();
  void formatFilterChanged();
  void tagFilterChanged();
  void favoritesOnlyChanged();
  void tagsChanged();
  void sortOrderChanged();
  void isScanningChanged();
  void scanStateChanged();
  void scanProgressChanged();
  void totalStorageSizeChanged();
  void selectionChanged();
  void canUndoTrashChanged();
  void lastTrashedChanged();
  void errorMessageChanged();
  void itemDeleted(const QString &path);
  void itemTrashed(const QString &path);
  void itemRestored(const QString &path);
  void thumbnailCacheCleared(bool success);
  void operationCompleted(const QString &message);
  void duplicateScanFinished(int groupsFound, qint64 totalReclaimable);

private slots:
  void onDirectoryChanged(const QString &path);
  void onSaveDirectorySettingChanged();
  void onScanProgress(int count);
  void onScanFinished(const ro_screenshot::ScanResult &result);

private:
  void filterAndSortItems();
  QString formatFileSize(qint64 bytes) const;
  bool passesFilter(const ScreenshotItem &item) const;
  void updateWatchers(const QStringList &directories);
  void loadMetadata();
  void saveMetadata();

  SettingsManager *m_settings{nullptr};
  QFileSystemWatcher m_watcher;
  std::shared_ptr<ThumbnailCache> m_cache;
  TrashManager m_trashManager;

  QList<ScreenshotItem> m_allItems;
  QList<ScreenshotItem> m_filteredItems;
  QSet<QString> m_selectedPaths;

  // Metadata stores: tags and favorites mapped by file path
  QMap<QString, QStringList> m_itemTags;
  QSet<QString> m_favoritePaths;

  QString m_searchQuery;
  int m_dateFilter{
      0}; // 0=All, 1=Today, 2=Yesterday, 3=ThisWeek, 4=Last7Days, 5=ThisMonth
  int m_formatFilter{0}; // 0=All, 1=PNG, 2=JPG, 3=WEBP
  QString m_tagFilter;
  bool m_favoritesOnly{false};
  int m_sortOrder{0}; // NewestFirst

  bool m_isScanning{false};
  QString m_scanState{"idle"}; // "idle", "scanning", "error"
  int m_scanProgress{0};
  qint64 m_totalBytes{0};
  QString m_errorMessage;
  QString m_lastTrashedFileName;

  uint64_t m_currentGeneration{0};
  std::shared_ptr<std::atomic<bool>> m_currentCancelFlag;
};

} // namespace ro_screenshot
