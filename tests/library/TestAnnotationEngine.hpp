#pragma once

#include "core/LibraryManager.hpp"
#include "core/SettingsManager.hpp"
#include "core/library/AnnotationEngine.hpp"
#include <QBuffer>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QTemporaryDir>
#include <QTest>

namespace ro_screenshot {

class TestAnnotationEngine : public QObject {
  Q_OBJECT

public:
  static void runAllTests() {
    testAnnotationCommandsUndoRedo();
    testAnnotationRenderingAndExport();
    testQmlOverlayExportKeepsOriginalResolution();
  }

  static void testAnnotationCommandsUndoRedo() {
    AnnotationEngine engine;

    QImage base(800, 600, QImage::Format_ARGB32_Premultiplied);
    base.fill(Qt::white);
    engine.setBaseImage(base);

    QCOMPARE(engine.annotationCount(), 0);
    QCOMPARE(engine.canUndo(), false);
    QCOMPARE(engine.canRedo(), false);

    // 1. Add Rectangle
    engine.addRectangle(50, 50, 200, 100, true);
    QCOMPARE(engine.annotationCount(), 1);
    QCOMPARE(engine.canUndo(), true);

    // 2. Add Arrow
    engine.addArrow(10, 10, 100, 100);
    QCOMPARE(engine.annotationCount(), 2);

    // 3. Add Step Marker
    engine.addStepMarker(300, 300);
    QCOMPARE(engine.annotationCount(), 3);

    // 4. Undo Step Marker
    engine.undo();
    QCOMPARE(engine.annotationCount(), 2);
    QCOMPARE(engine.canRedo(), true);

    // 5. Undo Arrow
    engine.undo();
    QCOMPARE(engine.annotationCount(), 1);

    // 6. Redo Arrow
    engine.redo();
    QCOMPARE(engine.annotationCount(), 2);
  }

  static void testAnnotationRenderingAndExport() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    AnnotationEngine engine;
    QImage base(400, 300, QImage::Format_ARGB32_Premultiplied);
    base.fill(Qt::black);
    engine.setBaseImage(base);

    // Add rectangle, text and blur
    engine.addRectangle(20, 20, 100, 100);
    engine.addText(150, 50, "Test Annotation", 16);
    engine.addBlur(200, 200, 50, 50, 10);

    // Render flattened
    QImage result = engine.renderFlattened();
    QVERIFY(!result.isNull());
    QCOMPARE(result.width(), 400);
    QCOMPARE(result.height(), 300);

    // Export to file
    QString outPath = tempDir.path() + "/exported.png";
    QVERIFY(engine.exportToFile(outPath, "PNG"));
    QVERIFY(QFile::exists(outPath));

    QImage loaded(outPath);
    QCOMPARE(loaded.size(), QSize(400, 300));
  }

  static void testQmlOverlayExportKeepsOriginalResolution() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SettingsManager settings;
    settings.setSaveDirectory(tempDir.path());
    const QString sourcePath = tempDir.path() + "/source.png";
    QImage source(800, 400, QImage::Format_ARGB32_Premultiplied);
    source.fill(Qt::white);
    QVERIFY(source.save(sourcePath, "PNG"));

    QImage overlay(200, 100, QImage::Format_ARGB32_Premultiplied);
    overlay.fill(Qt::transparent);
    {
      QPainter painter(&overlay);
      painter.fillRect(0, 0, 100, 50, Qt::red);
    }
    QBuffer buffer;
    QVERIFY(buffer.open(QIODevice::WriteOnly));
    QVERIFY(overlay.save(&buffer, "PNG"));
    const QString dataUrl = QStringLiteral("data:image/png;base64,") +
                            QString::fromLatin1(buffer.data().toBase64());

    LibraryManager library(&settings);
    const QVariantMap saved = library.exportAnnotatedImage(
        sourcePath, dataUrl, 0, 0, overlay.width(), overlay.height());
    QVERIFY(saved.value("success").toBool());
    const QImage exported(saved.value("path").toString());
    QCOMPARE(exported.size(), source.size());
    QCOMPARE(exported.pixelColor(100, 100), QColor(Qt::red));
    QCOMPARE(QImage(sourcePath).pixelColor(100, 100), QColor(Qt::white));
  }
};

} // namespace ro_screenshot
