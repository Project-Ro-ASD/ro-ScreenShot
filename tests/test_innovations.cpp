#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>

#include "core/advanced/AccessibilityAnalyzer.hpp"
#include "core/advanced/AiAssistantEngine.hpp"
#include "core/advanced/DuplicateCleaner.hpp"
#include "core/advanced/EphemeralShotManager.hpp"
#include "core/advanced/ImageDiffEngine.hpp"
#include "core/advanced/LivePresentationOverlay.hpp"
#include "core/advanced/LiveTranslator.hpp"
#include "core/advanced/MetadataSanitizer.hpp"
#include "core/advanced/MockupFrameGenerator.hpp"
#include "core/advanced/PaletteExtractor.hpp"
#include "core/advanced/PdfReportGenerator.hpp"
#include "core/advanced/PerceptualHashEngine.hpp"
#include "core/advanced/QrCodeDetector.hpp"
#include "core/advanced/ScreenRecorderEngine.hpp"
#include "core/advanced/ScrollingCaptureEngine.hpp"
#include "core/advanced/SensitiveDataDetector.hpp"
#include "core/advanced/SmartDimensionGuide.hpp"
#include "core/advanced/SmartWindowDetector.hpp"
#include "core/advanced/TableExtractor.hpp"
#include "core/advanced/VaultManager.hpp"
#include "core/library/AnnotationEngine.hpp"
#include "core/platform/WlrScreencopyProvider.hpp"

using namespace ro_screenshot;

class TestInnovations : public QObject {
  Q_OBJECT

private slots:
  void testSensitiveDataDetection();
  void testMockupFrameGeneration();
  void testPaletteExtraction();
  void testSmartDimensionGuide();
  void testTableExtraction();
  void testVaultManager();
  void testEphemeralShotManager();
  void testMetadataSanitization();
  void testPerceptualHashEngine();
  void testDuplicateCleaner();
  void testAccessibilityAndColorBlindness();
  void testPdfReportGenerator();
  void testImageDiffEngine();
  void testLiveTranslator();
  void testScrollingCaptureStitching();
  void testSmartWindowDetector();
  void testLivePresentationOverlay();
  void testQrCodeSmartActionParsing();
  void testScreenRecorderEngineProperties();
  void testAnnotationEngineInnovations();
};

void TestInnovations::testSensitiveDataDetection() {
  // Test TCKN validation
  QVERIFY(!SensitiveDataDetector::validateTurkishId(
      "12345678901")); // Invalid checksum
  QVERIFY(!SensitiveDataDetector::validateTurkishId(
      "01234567890")); // Starts with 0

  // Known valid Turkish ID algorithm test
  // 10000000146 -> oddSum=1+0+0+0+1=2, evenSum=0+0+0+0=0, d10=(14-0)%10=4.
  // sum=1+0+0+0+0+0+0+0+1+4=6, d11=6%10=6.
  QVERIFY(SensitiveDataDetector::validateTurkishId("10000000146"));

  // Test Luhn Credit Card validation
  QVERIFY(SensitiveDataDetector::validateLuhnCreditCard(
      "4532015112830366")); // Valid Luhn
  QVERIFY(!SensitiveDataDetector::validateLuhnCreditCard(
      "4532015112830367")); // Invalid Luhn

  // Test sensitive data detection in text
  QString sampleText =
      "Kullanıcı TCKN: 10000000146, Email: emir@example.com, Tel: 0532 123 45 "
      "67, Token: ghp_123456789012345678901234567890123456";
  auto matches = SensitiveDataDetector::detect(sampleText);
  QVERIFY(matches.size() >= 3);
}

void TestInnovations::testMockupFrameGeneration() {
  QImage testImg(400, 300, QImage::Format_ARGB32);
  testImg.fill(QColor("#3B82F6"));

  MockupOptions opt;
  opt.preset = MockupPreset::Sunset;
  opt.padding = 32;
  opt.drawWindowControls = true;

  QImage framed = MockupFrameGenerator::generate(testImg, opt);
  QVERIFY(!framed.isNull());
  QCOMPARE(framed.width(), 400 + 64);
  QCOMPARE(framed.height(), 300 + 32 + 64);
}

void TestInnovations::testPaletteExtraction() {
  QImage testImg(100, 100, QImage::Format_ARGB32);
  testImg.fill(QColor("#10B981")); // Emerald

  auto palette = PaletteExtractor::extract(testImg, 4);
  QVERIFY(!palette.isEmpty());
  QVERIFY(palette[0].hex.startsWith("#"));
  QVERIFY(!palette[0].tailwindClass.isEmpty());

  QString css = PaletteExtractor::exportAsCssVariables(palette);
  QVERIFY(css.contains("--color-palette-1"));

  QString json = PaletteExtractor::exportAsJson(palette);
  QVERIFY(json.contains("dominancePercent"));
}

void TestInnovations::testSmartDimensionGuide() {
  QRect elA(10, 10, 100, 50);
  QRect elB(150, 10, 100, 50);

  auto m = SmartDimensionGuide::measure(elA, elB);
  QCOMPARE(m.marginRight, 40); // 150 - 110 = 40

  QSize screen(1920, 1080);
  auto mScreen = SmartDimensionGuide::measureAgainstScreen(elA, screen);
  QCOMPARE(mScreen.marginLeft, 10);
  QCOMPARE(mScreen.marginTop, 10);
}

void TestInnovations::testTableExtraction() {
  QString rawData = "Header 1\tHeader 2\nValue 1\tValue 2\nValue 3\tValue 4";
  ExtractedTable tbl = TableExtractor::extractFromText(rawData);
  QVERIFY(tbl.isValid);
  QCOMPARE(tbl.rowCount, 3);
  QCOMPARE(tbl.columnCount, 2);

  QString md = TableExtractor::toMarkdown(tbl);
  QVERIFY(md.contains("| Header 1 | Header 2 |"));

  QString csv = TableExtractor::toCsv(tbl);
  QVERIFY(csv.contains("Value 1,Value 2"));
}

void TestInnovations::testVaultManager() {
  QTemporaryDir tempDir;
  QVERIFY(tempDir.isValid());

  VaultManager vault;
  vault.setVaultDirectory(tempDir.path());

  QVERIFY(!vault.isUnlocked());
  QVERIFY(vault.setInitialPassword("SecretPass123!"));
  QVERIFY(vault.isUnlocked());

  // Create sample image and encrypt into vault
  QImage sample(100, 100, QImage::Format_RGB32);
  sample.fill(Qt::red);
  QString samplePath = tempDir.path() + "/sample.png";
  sample.save(samplePath);

  QVERIFY(vault.encryptAndStoreImage(samplePath));
  QCOMPARE(vault.vaultItemCount(), 1);

  auto items = vault.listVaultItems();
  QCOMPARE(items.size(), 1);

  QImage decrypted = vault.decryptImage(items[0].vaultFilePath);
  QVERIFY(!decrypted.isNull());
  QCOMPARE(decrypted.size(), QSize(100, 100));

  // Test locking and wrong password
  vault.lock();
  QVERIFY(!vault.isUnlocked());
  QVERIFY(!vault.unlock("WrongPass"));
  QVERIFY(vault.unlock("SecretPass123!"));
  QVERIFY(vault.isUnlocked());
}

void TestInnovations::testEphemeralShotManager() {
  QTemporaryDir tempDir;
  QVERIFY(tempDir.isValid());

  QString testFile = tempDir.path() + "/ephemeral.png";
  QImage(10, 10, QImage::Format_Mono).save(testFile);

  EphemeralShotManager manager;
  manager.registerEphemeralFile(testFile, 1); // 1 sec TTL
  QVERIFY(manager.isEphemeral(testFile));
  QCOMPARE(manager.activeEphemeralCount(), 1);

  QTest::qWait(1200);
  manager.checkAndPurgeExpired();

  QVERIFY(!QFile::exists(testFile));
  QCOMPARE(manager.activeEphemeralCount(), 0);
}

void TestInnovations::testMetadataSanitization() {
  QTemporaryDir tempDir;
  QVERIFY(tempDir.isValid());

  QString testImgPath = tempDir.path() + "/clean_test.png";
  QImage img(200, 200, QImage::Format_ARGB32);
  img.fill(Qt::blue);
  img.setText("Software", "TestAuthor");
  img.save(testImgPath);

  QVERIFY(MetadataSanitizer::sanitizeFile(testImgPath));

  QImage reloaded(testImgPath);
  QVERIFY(reloaded.text("Software").isEmpty());

  auto stats = MetadataSanitizer::optimizeLossless(testImgPath);
  QVERIFY(stats.success);
}

void TestInnovations::testPerceptualHashEngine() {
  QImage imgA(200, 200, QImage::Format_RGB32);
  imgA.fill(Qt::yellow);

  QImage imgB = imgA.copy();
  // Slightly modify 1 pixel
  imgB.setPixelColor(10, 10, Qt::black);

  quint64 hashA = PerceptualHashEngine::computeDHash(imgA);
  quint64 hashB = PerceptualHashEngine::computeDHash(imgB);

  int dist = PerceptualHashEngine::hammingDistance(hashA, hashB);
  QVERIFY(dist <= 4);
  QVERIFY(PerceptualHashEngine::areDuplicates(hashA, hashB, 6));
}

void TestInnovations::testDuplicateCleaner() {
  QTemporaryDir tempDir;
  QVERIFY(tempDir.isValid());

  QString path1 = tempDir.path() + "/orig.png";
  QString path2 = tempDir.path() + "/dup.png";

  QImage img(100, 100, QImage::Format_RGB32);
  img.fill(Qt::cyan);
  img.save(path1);
  img.save(path2);

  auto groups = DuplicateCleaner::findDuplicates({path1, path2}, 5);
  QCOMPARE(groups.size(), 1);
  QCOMPARE(groups[0].duplicateFilePaths.size(), 1);
}

void TestInnovations::testAccessibilityAndColorBlindness() {
  QColor fg(Qt::black);
  QColor bg(Qt::white);

  auto wcag = AccessibilityAnalyzer::evaluateContrast(fg, bg);
  QVERIFY(wcag.ratio > 20.0);
  QVERIFY(wcag.passesAaaNormal);
  QCOMPARE(wcag.grade, QString("AAA"));

  QImage source(50, 50, QImage::Format_ARGB32);
  source.fill(Qt::red);

  QImage protanopia = AccessibilityAnalyzer::simulateColorBlindness(
      source, ColorBlindnessType::Protanopia);
  QVERIFY(!protanopia.isNull());
}

void TestInnovations::testPdfReportGenerator() {
  QTemporaryDir tempDir;
  QVERIFY(tempDir.isValid());

  QString imgPath = tempDir.path() + "/shot.png";
  QImage(100, 100, QImage::Format_RGB32).save(imgPath);

  PdfReportEntry entry;
  entry.imagePath = imgPath;
  entry.title = "Test Bug Screen";
  entry.description = "Regression in panel";

  PdfReportOptions opt;
  opt.title = "Automated Test Report";

  QString outPdf = tempDir.path() + "/report.pdf";
  bool ok = PdfReportGenerator::generateReport(outPdf, {entry}, opt);
  QVERIFY(ok);
  QVERIFY(QFile::exists(outPdf));
  QVERIFY(QFileInfo(outPdf).size() > 1000);
}

void TestInnovations::testImageDiffEngine() {
  QImage imgA(100, 100, QImage::Format_RGB32);
  imgA.fill(Qt::white);

  QImage imgB(100, 100, QImage::Format_RGB32);
  imgB.fill(Qt::white);
  // Red block on imgB
  for (int y = 20; y < 40; ++y) {
    for (int x = 20; x < 40; ++x) {
      imgB.setPixelColor(x, y, Qt::red);
    }
  }

  DiffResult diff = ImageDiffEngine::compare(imgA, imgB);
  QVERIFY(diff.dimensionsMatch);
  QCOMPARE(diff.differentPixels, 400);
  QVERIFY(diff.differencePercent > 0.0);
  QVERIFY(!diff.diffImage.isNull());

  QImage split = ImageDiffEngine::renderSplitView(imgA, imgB, 0.5);
  QVERIFY(!split.isNull());
}

void TestInnovations::testLiveTranslator() {
  LiveTranslator translator;
  QString translated = translator.translateText("error", "en", "tr");
  QCOMPARE(translated.toLower(), QString("hata"));
}

void TestInnovations::testScrollingCaptureStitching() {
  QImage frame1(200, 100, QImage::Format_RGB32);
  frame1.fill(Qt::white);

  QImage frame2(200, 100, QImage::Format_RGB32);
  frame2.fill(Qt::white);

  QVector<QImage> frames = {frame1, frame2};
  QImage stitched = ScrollingCaptureEngine::stitchFrames(frames);
  QVERIFY(!stitched.isNull());
  QVERIFY(stitched.height() >= 100);
}

void TestInnovations::testSmartWindowDetector() {
  QVector<QRect> windows = {QRect(100, 100, 400, 300)};
  QSize screen(1920, 1080);

  // Cursor near top-left of window (102, 105)
  QRect snapped = SmartWindowDetector::snapToWindowOrEdge(QPoint(102, 105),
                                                          windows, screen);
  QCOMPARE(snapped, QRect(100, 100, 400, 300));
}

void TestInnovations::testLivePresentationOverlay() {
  LivePresentationOverlay overlay;
  overlay.setLaserActive(true);
  overlay.addLaserPoint(100.0, 200.0);
  QVERIFY(overlay.isLaserActive());

  overlay.recordKeystroke("Ctrl+Shift+S");
  QCOMPARE(overlay.currentKeystroke(), QString("Ctrl+Shift+S"));
}

void TestInnovations::testQrCodeSmartActionParsing() {
  // Test Wi-Fi parsing
  auto wifiAction =
      QrCodeDetector::parsePayload("WIFI:S:MyNetwork;T:WPA;P:Secret123;;");
  QCOMPARE(wifiAction.actionType, QrActionType::ConnectWifi);
  QCOMPARE(wifiAction.wifiSsid, QString("MyNetwork"));
  QCOMPARE(wifiAction.wifiPassword, QString("Secret123"));

  // Test URL parsing
  auto urlAction = QrCodeDetector::parsePayload(
      "https://github.com/Project-Ro-ASD/Ro-ScreenShot");
  QCOMPARE(urlAction.actionType, QrActionType::OpenUrl);
  QCOMPARE(urlAction.targetUrl,
           QString("https://github.com/Project-Ro-ASD/Ro-ScreenShot"));
}

void TestInnovations::testScreenRecorderEngineProperties() {
  ScreenRecorderEngine engine;
  QVERIFY(!engine.isRecording());
  QCOMPARE(engine.formattedDuration(), QString("00:00"));

  engine.setMicEnabled(true);
  QVERIFY(engine.micEnabled());

  engine.setSystemAudioEnabled(true);
  QVERIFY(engine.systemAudioEnabled());
}

void TestInnovations::testAnnotationEngineInnovations() {
  AnnotationEngine engine;
  QImage base(300, 300, QImage::Format_ARGB32);
  base.fill(Qt::white);
  engine.setBaseImage(base);

  // Add Step Marker
  engine.addStepMarker(50, 50);
  QCOMPARE(engine.annotationCount(), 1);

  // Add Status Badge
  engine.addStatusBadge(100, 100, StatusBadgeType::Checkmark);
  QCOMPARE(engine.annotationCount(), 2);

  // Add Callout Loupe
  engine.addCalloutLoupe(150, 150, 40, 2.0);
  QCOMPARE(engine.annotationCount(), 3);

  QImage rendered = engine.renderFlattened();
  QVERIFY(!rendered.isNull());
  QCOMPARE(rendered.size(), QSize(300, 300));
}

QTEST_MAIN(TestInnovations)
#include "test_innovations.moc"
