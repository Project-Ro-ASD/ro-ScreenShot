#include "core/LibraryManager.hpp"
#include "core/SettingsManager.hpp"
#include "library/TestAnnotationEngine.hpp"
#include "library/TestGalleryBackend.hpp"
#include "library/TestSafeOperations.hpp"
#include <QDir>
#include <QImage>
#include <QSettings>
#include <QTemporaryDir>
#include <QTest>

using namespace ro_screenshot;

class TestLibrary : public QObject {
  Q_OBJECT

  QTemporaryDir m_configDir;

private slots:
  void initTestCase() {
    QVERIFY(m_configDir.isValid());
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope,
                       m_configDir.path());
  }

  void testScanAndFilter() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SettingsManager settings;
    settings.setSaveDirectory(tempDir.path());

    // Create 2 test images
    QImage img1(800, 600, QImage::Format_RGB32);
    img1.fill(Qt::red);
    QVERIFY(img1.save(tempDir.path() + "/test_shot_alpha.png", "PNG"));

    QImage img2(1920, 1080, QImage::Format_RGB32);
    img2.fill(Qt::blue);
    QVERIFY(img2.save(tempDir.path() + "/test_shot_beta.png", "PNG"));

    LibraryManager library(&settings);
    library.refresh();
    QVERIFY(library.waitForScan());

    QCOMPARE(library.count(), 2);

    // Search query filter test
    library.setSearchQuery("alpha");
    QCOMPARE(library.count(), 1);
    QCOMPARE(library.data(library.index(0, 0), LibraryManager::FileNameRole)
                 .toString(),
             QString("test_shot_alpha.png"));

    library.setSearchQuery("");
    QCOMPARE(library.count(), 2);

    // Test deletion (moves to trash)
    QVERIFY(library.deleteItem(0));
    QVERIFY(library.waitForScan());
    QCOMPARE(library.count(), 1);
  }

  void testGalleryBackendSuite() { TestGalleryBackend::runAllTests(); }

  void testSafeOperationsSuite() { TestSafeOperations::runAllTests(); }

  void testAnnotationEngineSuite() { TestAnnotationEngine::runAllTests(); }
};

QTEST_MAIN(TestLibrary)
#include "test_library.moc"
