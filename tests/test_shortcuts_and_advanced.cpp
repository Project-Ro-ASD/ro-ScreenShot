#include "core/DesktopFeedback.hpp"
#include "core/advanced/AutostartManager.hpp"
#include "core/advanced/CloudShareManager.hpp"
#include "core/advanced/OcrEngine.hpp"
#include "core/advanced/QrCodeDetector.hpp"
#include "core/advanced/ScreenRecorderEngine.hpp"
#include "core/shortcuts/ShortcutManager.hpp"
#include <QColor>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

using namespace ro_screenshot;

class TestShortcutsAndAdvanced : public QObject {
  Q_OBJECT

private slots:
  void testShortcutManagerDefinitions() {
    ShortcutManager mgr;
    const auto list = mgr.shortcuts();
    QCOMPARE(list.size(), 12);

    // Verify key shortcut actions exist
    QCOMPARE(mgr.keySequence(ShortcutAction::CaptureRegion),
             QStringLiteral("Ctrl+Shift+Print"));
    QCOMPARE(mgr.keySequence(ShortcutAction::CaptureFullscreen),
             QStringLiteral("Print"));
    QCOMPARE(mgr.keySequence(ShortcutAction::CaptureWindow),
             QStringLiteral("Alt+Print"));
    QCOMPARE(mgr.keySequence(ShortcutAction::CaptureLastRegion),
             QStringLiteral("Shift+Print"));
    QCOMPARE(mgr.keySequence(ShortcutAction::ColorPicker),
             QStringLiteral("Meta+Shift+C"));
    QCOMPARE(mgr.keySequence(ShortcutAction::ToggleRecording),
             QStringLiteral("Ctrl+Shift+R"));
  }

  void testShortcutConflictDetection() {
    ShortcutManager mgr;
    mgr.resetToDefaults();

    // No conflicts initially
    QCOMPARE(mgr.allConflicts().size(), 0);

    // Try setting region shortcut to match fullscreen (Print)
    ShortcutAction conflict;
    const bool hasConf =
        mgr.hasConflict(ShortcutAction::CaptureRegion, "Print", &conflict);
    QVERIFY(hasConf);
    QCOMPARE(conflict, ShortcutAction::CaptureFullscreen);

    // Change sequence and verify
    mgr.setKeySequence(ShortcutAction::CaptureRegion, "Ctrl+F12");
    QCOMPARE(mgr.keySequence(ShortcutAction::CaptureRegion),
             QStringLiteral("Ctrl+F12"));

    // Reset to defaults
    mgr.resetToDefaults();
    QCOMPARE(mgr.keySequence(ShortcutAction::CaptureRegion),
             QStringLiteral("Ctrl+Shift+Print"));
  }

  void testShortcutConfigGenerators() {
    ShortcutManager mgr;
    const QString kde = mgr.generateKdeShortcutsConfig();
    QVERIFY(kde.contains("org.ro_asd.ro_screenshot"));
    QVERIFY(kde.contains("ro-screenshot --region"));

    const QString gnome = mgr.generateGnomeShortcutsScript();
    QVERIFY(gnome.contains("custom-keybindings"));
    QVERIFY(gnome.contains("ro-screenshot --fullscreen"));

    const QString hypr = mgr.generateHyprlandConfig();
    QVERIFY(hypr.contains("bind ="));
    QVERIFY(hypr.contains("ro-screenshot --last-region"));

    const QString sway = mgr.generateSwayConfig();
    QVERIFY(sway.contains("bindsym"));
    QVERIFY(sway.contains("ro-screenshot --library"));
  }

  void testDesktopFeedbackColorFormats() {
    const QColor redColor(255, 0, 0);
    const ColorSample sample = DesktopFeedback::sampleColor(redColor);
    QCOMPARE(sample.hex, QStringLiteral("#FF0000"));
    QCOMPARE(sample.rgb, QStringLiteral("rgb(255, 0, 0)"));
    QCOMPARE(sample.hsl, QStringLiteral("hsl(0, 100%, 50%)"));

    QCOMPARE(DesktopFeedback::formatColor(redColor, ColorFormat::Hex),
             QStringLiteral("#FF0000"));
    QCOMPARE(DesktopFeedback::formatColor(redColor, ColorFormat::Rgb),
             QStringLiteral("rgb(255, 0, 0)"));
    QCOMPARE(DesktopFeedback::formatColor(redColor, ColorFormat::Hsl),
             QStringLiteral("hsl(0, 100%, 50%)"));
  }

  void testSecureCredentialStore() {
    SecureCredentialStore store;
    const QString service = "Nextcloud";
    const QString account = "user@test.org";
    const QString secret = "super_secret_token_12345";

    QVERIFY(store.storeSecret(service, account, secret));
    const QString retrieved = store.retrieveSecret(service, account);
    QCOMPARE(retrieved, secret);

    QVERIFY(store.deleteSecret(service, account));
    QVERIFY(store.retrieveSecret(service, account).isEmpty());
  }

  void testAutostartManager() {
    AutostartManager autostart;
    const QString path = autostart.autostartFilePath();
    QVERIFY(path.contains("autostart"));

    QVERIFY(autostart.setAutostartEnabled(true));
    QVERIFY(autostart.isAutostartEnabled());
    QVERIFY(QFile::exists(path));

    QVERIFY(autostart.setAutostartEnabled(false));
    QVERIFY(!autostart.isAutostartEnabled());
    QVERIFY(!QFile::exists(path));
  }

  void testScreenRecorderEngineStateMachine() {
    ScreenRecorderEngine recorder;
    QCOMPARE(recorder.state(), ScreenRecorderEngine::RecordingState::Idle);
    QCOMPARE(recorder.isRecording(), false);

    ScreenRecorderEngine::RecordingOptions opt;
    opt.outputPath = "/tmp/test_rec.mp4";

    QVERIFY(recorder.startRecording(opt));
    QCOMPARE(recorder.state(), ScreenRecorderEngine::RecordingState::Recording);
    QVERIFY(recorder.isRecording());

    QVERIFY(recorder.pauseRecording());
    QCOMPARE(recorder.state(), ScreenRecorderEngine::RecordingState::Paused);
    QVERIFY(recorder.isPaused());

    QVERIFY(recorder.resumeRecording());
    QCOMPARE(recorder.state(), ScreenRecorderEngine::RecordingState::Recording);

    QVERIFY(recorder.stopRecording());
    QCOMPARE(recorder.state(), ScreenRecorderEngine::RecordingState::Idle);
  }

  void testOcrEngineLanguages() {
    OcrEngine ocr;
    const auto langs = ocr.availableLanguages();
    QVERIFY(!langs.isEmpty());
    ocr.setDefaultLanguage("tur");
    QCOMPARE(ocr.defaultLanguage(), QStringLiteral("tur"));
  }
};

QTEST_MAIN(TestShortcutsAndAdvanced)
#include "test_shortcuts_and_advanced.moc"
