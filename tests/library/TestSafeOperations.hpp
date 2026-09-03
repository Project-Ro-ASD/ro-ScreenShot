#pragma once

#include "core/LibraryManager.hpp"
#include "core/SettingsManager.hpp"
#include "core/library/TrashManager.hpp"
#include <QDir>
#include <QFile>
#include <QImage>
#include <QTemporaryDir>
#include <QTest>

namespace ro_screenshot {

class TestSafeOperations : public QObject {
  Q_OBJECT

public:
  static void runAllTests() {
    testSafeTrashBoundaryChecks();
    testTrashAndUndo();
    testRenameCollisionAndValidation();
    testMultiSelectionBehavior();
  }

  static void testSafeTrashBoundaryChecks() {
    QTemporaryDir libraryDir;
    QVERIFY(libraryDir.isValid());

    QTemporaryDir outsideDir;
    QVERIFY(outsideDir.isValid());

    // Create outside file
    QString outsidePath = outsideDir.path() + "/critical_file.txt";
    QFile f(outsidePath);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("important");
    f.close();

    TrashManager trashManager;
    // Boundary check must fail for file outside allowed root
    QVERIFY(!trashManager.isPathSafe(outsidePath, libraryDir.path()));

    QString errorMsg;
    QVERIFY(!trashManager.trashFile(outsidePath, libraryDir.path(), &errorMsg));
    QVERIFY(QFile::exists(outsidePath)); // File must not be deleted
  }

  static void testTrashAndUndo() {
    QTemporaryDir libraryDir;
    QVERIFY(libraryDir.isValid());

    SettingsManager settings;
    settings.setSaveDirectory(libraryDir.path());

    // Create a test image
    QString imgPath = libraryDir.path() + "/trash_me.png";
    QImage img(100, 100, QImage::Format_RGB32);
    img.fill(Qt::red);
    QVERIFY(img.save(imgPath, "PNG"));

    LibraryManager library(&settings);
    library.refresh();
    QVERIFY(library.waitForScan());
    QCOMPARE(library.count(), 1);

    // Delete (moves to Trash)
    QVERIFY(library.deleteItem(0));
    QVERIFY(library.waitForScan());
    QCOMPARE(library.count(), 0);
    QVERIFY(library.canUndoTrash());

    // Undo Trash
    QVERIFY(library.undoLastTrash());
    QVERIFY(library.waitForScan());
    QCOMPARE(library.count(), 1);
  }

  static void testRenameCollisionAndValidation() {
    QTemporaryDir libraryDir;
    QVERIFY(libraryDir.isValid());

    SettingsManager settings;
    settings.setSaveDirectory(libraryDir.path());

    QString img1Path = libraryDir.path() + "/original.png";
    QString img2Path = libraryDir.path() + "/existing.png";

    QImage img(100, 100, QImage::Format_RGB32);
    img.fill(Qt::blue);
    QVERIFY(img.save(img1Path, "PNG"));
    QVERIFY(img.save(img2Path, "PNG"));

    LibraryManager library(&settings);
    library.refresh();
    QVERIFY(library.waitForScan());
    QCOMPARE(library.count(), 2);

    // Attempt rename to existing name -> should fail with collision error
    QVariantMap res1 = library.renameItem(0, "existing");
    QCOMPARE(res1["success"].toBool(), false);

    // Attempt rename with invalid characters -> should fail
    QVariantMap res2 = library.renameItem(0, "bad/name:test*");
    QCOMPARE(res2["success"].toBool(), false);

    // Valid rename
    QVariantMap res3 = library.renameItem(0, "renamed_shot");
    QCOMPARE(res3["success"].toBool(), true);
  }

  static void testMultiSelectionBehavior() {
    QTemporaryDir libraryDir;
    QVERIFY(libraryDir.isValid());

    SettingsManager settings;
    settings.setSaveDirectory(libraryDir.path());

    for (int i = 0; i < 4; ++i) {
      QImage img(100, 100, QImage::Format_RGB32);
      img.fill(Qt::green);
      img.save(libraryDir.path() + QString("/select_%1.png").arg(i), "PNG");
    }

    LibraryManager library(&settings);
    library.refresh();
    QVERIFY(library.waitForScan());
    QCOMPARE(library.count(), 4);

    // Toggle selection
    library.toggleSelection(0);
    library.toggleSelection(2);
    QCOMPARE(library.selectedCount(), 2);
    QVERIFY(library.isSelected(0));
    QVERIFY(!library.isSelected(1));
    QVERIFY(library.isSelected(2));

    // Select all
    library.selectAll();
    QCOMPARE(library.selectedCount(), 4);

    // Clear selection
    library.clearSelection();
    QCOMPARE(library.selectedCount(), 0);
  }
};

} // namespace ro_screenshot
