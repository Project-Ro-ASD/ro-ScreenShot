#pragma once

#include "SettingsManager.hpp"
#include "Types.hpp"
#include <QAbstractListModel>
#include <QDateTime>
#include <QFileSystemWatcher>
#include <QFutureWatcher>
#include <QList>
#include <QObject>
#include <QString>

namespace ro_screenshot {

class LibraryManager : public QAbstractListModel {
  Q_OBJECT

  Q_PROPERTY(int count READ count NOTIFY countChanged)
  Q_PROPERTY(QString searchQuery READ searchQuery WRITE setSearchQuery NOTIFY
                 searchQueryChanged)
  Q_PROPERTY(int dateFilter READ dateFilter WRITE setDateFilter NOTIFY
                 dateFilterChanged)
  Q_PROPERTY(bool isScanning READ isScanning NOTIFY isScanningChanged)
  Q_PROPERTY(QString totalStorageSize READ totalStorageSize NOTIFY
                 totalStorageSizeChanged)

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
    ThumbnailUrlRole
  };
  Q_ENUM(Roles)

  explicit LibraryManager(SettingsManager *settings, QObject *parent = nullptr);
  ~LibraryManager() override = default;

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;

  int count() const;
  QString searchQuery() const;
  void setSearchQuery(const QString &query);

  int dateFilter() const;
  void setDateFilter(int filter);

  bool isScanning() const;
  QString totalStorageSize() const;

  Q_INVOKABLE void refresh();
  Q_INVOKABLE bool deleteItem(int row);
  Q_INVOKABLE bool deleteItemByPath(const QString &path);
  Q_INVOKABLE bool copyToClipboard(int row);
  Q_INVOKABLE void openInFolder(int row);
  Q_INVOKABLE void openFile(int row);
  Q_INVOKABLE QString getFilePath(int row) const;
  Q_INVOKABLE QVariantMap getItem(int row) const;

signals:
  void countChanged();
  void searchQueryChanged();
  void dateFilterChanged();
  void isScanningChanged();
  void totalStorageSizeChanged();
  void itemDeleted(const QString &path);

private slots:
  void onDirectoryChanged(const QString &path);
  void onSaveDirectorySettingChanged();

private:
  void filterItems();
  QString formatFileSize(qint64 bytes) const;
  QString getOrCreateThumbnail(const QString &imagePath) const;
  bool passesFilter(const ScreenshotItem &item) const;

  SettingsManager *m_settings{nullptr};
  QFileSystemWatcher m_watcher;
  QList<ScreenshotItem> m_allItems;
  QList<ScreenshotItem> m_filteredItems;
  QString m_searchQuery;
  int m_dateFilter{0}; // 0=All, 1=Today, 2=Yesterday, 3=ThisWeek, 4=ThisMonth
  bool m_isScanning{false};
  qint64 m_totalBytes{0};
  QString m_cacheDir;
};

} // namespace ro_screenshot
