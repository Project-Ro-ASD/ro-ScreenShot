#include "core/CaptureEngine.hpp"
#include "core/DBusService.hpp"
#include "core/DesktopFeedback.hpp"
#include "core/LanguageManager.hpp"
#include "core/LibraryManager.hpp"
#include "core/SettingsManager.hpp"
#include "core/Types.hpp"
#include "core/UiPreferencesManager.hpp"
#include "core/advanced/AutostartManager.hpp"
#include "core/advanced/SystemTrayManager.hpp"
#include "core/shortcuts/ShortcutManager.hpp"

#include <QBuffer>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QIcon>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlError>
#include <QQuickWindow>
#include <QTimer>
#include <QTranslator>
#include <iostream>

using namespace ro_screenshot;

int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);
  app.setApplicationName("ro-screenshot");
  app.setApplicationDisplayName("ro-ScreenShot");
  app.setOrganizationName("ro-asd");
  app.setOrganizationDomain("ro-asd.org");
  app.setApplicationVersion("1.0.0");

  QCommandLineParser parser;
  parser.setApplicationDescription(
      "ro-ScreenShot: Modern Linux & Wayland Screen Capture and Gallery Tool");
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption regionOption({"r", "region"},
                                  "Bölge seçimi (Sniper) modunda başlat.");
  QCommandLineOption fullscreenOption({"f", "fullscreen"},
                                      "Tam ekran yakalama modunda başlat.");
  QCommandLineOption windowOption({"w", "window"},
                                  "Pencere yakalama modunda başlat.");
  QCommandLineOption lastRegionOption(
      "last-region", "Önceki bölge sınırlarıyla yeniden yakala.");
  QCommandLineOption libraryOption({"l", "library"},
                                   "Galeri / Kütüphane görünümünü aç.");
  QCommandLineOption settingsOption({"s", "settings"},
                                    "Ayarlar görünümünü aç.");
  QCommandLineOption delayOption({"d", "delay"},
                                 "Yakalama öncesi gecikme süresi (saniye).",
                                 "saniye", "0");
  QCommandLineOption outputOption({"o", "output"},
                                  "Özel kayıt dosya veya dizin yolu.", "yol");
  QCommandLineOption formatOption("format", "Görsel formatı (png, jpg, webp).",
                                  "format");
  QCommandLineOption qualityOption("quality", "Görsel kalitesi (1-100).",
                                   "kalite");
  QCommandLineOption copyOnlyOption("copy-only",
                                    "Yalnızca panoya kopyala; diske kaydetme.");
  QCommandLineOption saveOnlyOption("save-only",
                                    "Yalnızca diske kaydet; panoya kopyalama.");
  QCommandLineOption stdoutOption(
      "stdout", "Görsel verisini standart çıktıya (stdout) aktar.");
  QCommandLineOption jsonOption(
      "json", "Sonuçları yapılandırılmış JSON olarak döndür.");
  QCommandLineOption colorPickerOption("color-picker",
                                       "Renk seçici damlalık modunu başlat.");
  QCommandLineOption genShortcutsOption(
      "generate-shortcuts",
      "Kısayol yapılandırmasını yazdır (gnome, hyprland, sway, kde).", "ortam");
  QCommandLineOption autostartOption(
      "autostart", "Başlangıçta çalışmayı ayarla (enable, disable).", "durum");
  QCommandLineOption trayOption("tray",
                                "Sistem tepsisinde arka planda başlat.");
  QCommandLineOption renderOption(
      "render-to", "Main pencereyi PNG olarak çiz ve çık (doc).", "file");

  parser.addOption(regionOption);
  parser.addOption(fullscreenOption);
  parser.addOption(windowOption);
  parser.addOption(lastRegionOption);
  parser.addOption(libraryOption);
  parser.addOption(settingsOption);
  parser.addOption(delayOption);
  parser.addOption(outputOption);
  parser.addOption(formatOption);
  parser.addOption(qualityOption);
  parser.addOption(copyOnlyOption);
  parser.addOption(saveOnlyOption);
  parser.addOption(stdoutOption);
  parser.addOption(jsonOption);
  parser.addOption(colorPickerOption);
  parser.addOption(genShortcutsOption);
  parser.addOption(autostartOption);
  parser.addOption(trayOption);
  parser.addOption(renderOption);

  parser.process(app);

  // Shortcut generation CLI handler
  if (parser.isSet(genShortcutsOption)) {
    ShortcutManager shortcutMgr;
    const QString target = parser.value(genShortcutsOption).toLower();
    if (target == "kde") {
      std::cout << shortcutMgr.generateKdeShortcutsConfig().toStdString();
    } else if (target == "gnome") {
      std::cout << shortcutMgr.generateGnomeShortcutsScript().toStdString();
    } else if (target == "hyprland") {
      std::cout << shortcutMgr.generateHyprlandConfig().toStdString();
    } else if (target == "sway") {
      std::cout << shortcutMgr.generateSwayConfig().toStdString();
    } else {
      std::cerr << "Geçersiz kısayol ortamı. Desteklenenler: kde, gnome, "
                   "hyprland, sway\n";
      return static_cast<int>(StructuredExitCode::InvalidArguments);
    }
    return static_cast<int>(StructuredExitCode::Success);
  }

  // Autostart CLI handler
  if (parser.isSet(autostartOption)) {
    AutostartManager autostartMgr;
    const QString action = parser.value(autostartOption).toLower();
    if (action == "enable") {
      const bool ok = autostartMgr.setAutostartEnabled(true);
      std::cout << (ok ? "Başlangıçta çalıştırma aktif edildi.\n"
                       : "Başlangıçta çalıştırma ayarlanamadı.\n");
      return ok ? static_cast<int>(StructuredExitCode::Success)
                : static_cast<int>(StructuredExitCode::GeneralError);
    } else if (action == "disable") {
      const bool ok = autostartMgr.setAutostartEnabled(false);
      std::cout << (ok ? "Başlangıçta çalıştırma devre dışı bırakıldı.\n"
                       : "Başlangıçta çalıştırma ayarlanamadı.\n");
      return ok ? static_cast<int>(StructuredExitCode::Success)
                : static_cast<int>(StructuredExitCode::GeneralError);
    } else {
      std::cerr << "Geçersiz autostart parametresi. Desteklenenler: enable, "
                   "disable\n";
      return static_cast<int>(StructuredExitCode::InvalidArguments);
    }
  }

  // Delay argument validation
  bool delayIsValid = false;
  const int delaySec = parser.value(delayOption).toInt(&delayIsValid);
  if (!delayIsValid || delaySec < 0 || delaySec > 3600) {
    qCritical("--delay must be an integer between 0 and 3600 seconds.");
    return static_cast<int>(StructuredExitCode::InvalidArguments);
  }

  if (parser.isSet(copyOnlyOption) && parser.isSet(saveOnlyOption)) {
    qCritical("--copy-only and --save-only cannot be used together.");
    return static_cast<int>(StructuredExitCode::InvalidArguments);
  }

  const QString captureAction =
      parser.isSet(copyOnlyOption)
          ? QStringLiteral("copy")
          : (parser.isSet(saveOnlyOption) ? QStringLiteral("save") : QString());

  const bool hasCaptureRequest =
      parser.isSet(regionOption) || parser.isSet(fullscreenOption) ||
      parser.isSet(windowOption) || parser.isSet(lastRegionOption) ||
      parser.isSet(colorPickerOption) || parser.isSet(delayOption);

  // Check if an instance is already running via D-Bus
  QDBusInterface iface("org.ro_asd.ScreenShot", "/org/ro_asd/ScreenShot",
                       "org.ro_asd.ScreenShot", QDBusConnection::sessionBus());
  if (iface.isValid()) {
    const auto dispatchCapture = [&iface, delaySec,
                                  captureAction](const QString &defaultMethod,
                                                 const QString &actionMethod) {
      if (captureAction.isEmpty()) {
        iface.call(QDBus::NoBlock, defaultMethod, delaySec);
      } else {
        iface.call(QDBus::NoBlock, actionMethod, delaySec, captureAction);
      }
    };

    if (parser.isSet(lastRegionOption)) {
      dispatchCapture(QStringLiteral("CaptureLastRegion"),
                      QStringLiteral("CaptureLastRegionWithAction"));
      return static_cast<int>(StructuredExitCode::Success);
    } else if (parser.isSet(regionOption) || parser.isSet(delayOption) ||
               parser.isSet(colorPickerOption)) {
      dispatchCapture(QStringLiteral("CaptureRegion"),
                      QStringLiteral("CaptureRegionWithAction"));
      return static_cast<int>(StructuredExitCode::Success);
    } else if (parser.isSet(fullscreenOption)) {
      dispatchCapture(QStringLiteral("CaptureFullscreen"),
                      QStringLiteral("CaptureFullscreenWithAction"));
      return static_cast<int>(StructuredExitCode::Success);
    } else if (parser.isSet(windowOption)) {
      dispatchCapture(QStringLiteral("CaptureWindow"),
                      QStringLiteral("CaptureWindowWithAction"));
      return static_cast<int>(StructuredExitCode::Success);
    } else if (parser.isSet(settingsOption)) {
      iface.call(QDBus::NoBlock, "OpenSettings");
      return static_cast<int>(StructuredExitCode::Success);
    } else {
      iface.call(QDBus::NoBlock, "OpenLibrary");
      return static_cast<int>(StructuredExitCode::Success);
    }
  }

  // First instance initialization
  SettingsManager settingsManager;

  // CLI overrides for settings
  if (parser.isSet(outputOption)) {
    const QString outVal = parser.value(outputOption);
    const QFileInfo fi(outVal);
    if (fi.isDir()) {
      settingsManager.setSaveDirectory(fi.absoluteFilePath());
    } else {
      settingsManager.setSaveDirectory(fi.absolutePath());
      settingsManager.setFileNameTemplate(fi.completeBaseName());
      if (!fi.suffix().isEmpty()) {
        settingsManager.setImageFormat(fi.suffix());
      }
    }
  }
  if (parser.isSet(formatOption)) {
    settingsManager.setImageFormat(parser.value(formatOption));
  }
  if (parser.isSet(qualityOption)) {
    bool qOk = false;
    int qVal = parser.value(qualityOption).toInt(&qOk);
    if (qOk && qVal >= 1 && qVal <= 100) {
      settingsManager.setJpegQuality(qVal);
    }
  }

  LibraryManager libraryManager(&settingsManager);
  CaptureEngine captureEngine(&settingsManager, &libraryManager);
  DesktopFeedback desktopFeedback;
  ShortcutManager shortcutManager;
  AutostartManager autostartManager;
  SystemTrayManager systemTrayManager;

  if (parser.isSet(trayOption)) {
    systemTrayManager.initTray();
  }

  DBusService dbusService(&captureEngine);
  if (!dbusService.registerService()) {
    qWarning() << "[dbus] Single-instance service registration failed; "
                  "continuing without D-Bus command routing.";
  }

  QTranslator translator;
  QQmlApplicationEngine engine;

  LanguageManager languageManager(&app, &engine, &translator);
  qmlRegisterSingletonInstance("ro_screenshot", 1, 0, "LanguageManager",
                               &languageManager);

  UiPreferencesManager uiPreferencesManager;
  qmlRegisterSingletonInstance("ro_screenshot", 1, 0, "UiPreferencesManager",
                               &uiPreferencesManager);

  engine.rootContext()->setContextProperty("settingsManager", &settingsManager);
  engine.rootContext()->setContextProperty("libraryManager", &libraryManager);
  engine.rootContext()->setContextProperty("captureEngine", &captureEngine);
  engine.rootContext()->setContextProperty("languageManager", &languageManager);
  engine.rootContext()->setContextProperty("shortcutManager", &shortcutManager);

  // Load Main Hub using modern QML module API
  QObject::connect(&engine, &QQmlEngine::warnings,
                   [](const QList<QQmlError> &warnings) {
                     for (const auto &warning : warnings) {
                       fprintf(stderr, "[qml] %s\n",
                               warning.toString().toUtf8().constData());
                     }
                   });
  engine.loadFromModule("ro_screenshot", "MainHub");
  if (engine.rootObjects().isEmpty()) {
    return static_cast<int>(StructuredExitCode::GeneralError);
  }

  QObject *rootObject = engine.rootObjects().first();
  QQuickWindow *mainWindow = qobject_cast<QQuickWindow *>(rootObject);
  const bool captureOnlyLaunch = hasCaptureRequest;

  // Hide UI on capture
  QObject::connect(&captureEngine, &CaptureEngine::captureUiShouldHide, &app,
                   [mainWindow]() {
                     if (mainWindow) {
                       mainWindow->hide();
                     }
                   });

  // Output handling for stdout and JSON
  const bool wantsStdout = parser.isSet(stdoutOption);
  const bool wantsJson = parser.isSet(jsonOption);

  QObject::connect(&captureEngine, &CaptureEngine::captureSuccess, &app,
                   [wantsStdout, wantsJson, &captureEngine](
                       const QString &path, const QString &fileName, bool saved,
                       bool copied) {
                     if (wantsJson) {
                       const QImage img = captureEngine.lastCapturedImage();
                       QJsonObject jsonObj;
                       jsonObj["success"] = true;
                       jsonObj["filePath"] = path;
                       jsonObj["fileName"] = fileName;
                       jsonObj["savedToDisk"] = saved;
                       jsonObj["copiedToClipboard"] = copied;
                       jsonObj["width"] = img.width();
                       jsonObj["height"] = img.height();
                       jsonObj["timestamp"] =
                           QDateTime::currentDateTime().toString(Qt::ISODate);
                       std::cout << QJsonDocument(jsonObj)
                                        .toJson(QJsonDocument::Compact)
                                        .toStdString()
                                 << "\n";
                     }
                     if (wantsStdout) {
                       const QImage img = captureEngine.lastCapturedImage();
                       QByteArray bytes;
                       QBuffer buffer(&bytes);
                       buffer.open(QIODevice::WriteOnly);
                       img.save(&buffer, "PNG");
                       std::cout.write(bytes.constData(), bytes.size());
                       std::cout.flush();
                     }
                   });

  QObject::connect(&captureEngine, &CaptureEngine::captureErrorCode, &app,
                   [wantsJson](const QString &msg, CaptureErrorCode code) {
                     if (wantsJson) {
                       QJsonObject jsonObj;
                       jsonObj["success"] = false;
                       jsonObj["errorCode"] = static_cast<int>(code);
                       jsonObj["errorMessage"] = msg;
                       std::cout << QJsonDocument(jsonObj)
                                        .toJson(QJsonDocument::Compact)
                                        .toStdString()
                                 << "\n";
                     }
                   });

  QObject::connect(&captureEngine, &CaptureEngine::captureUiMayRestore, &app,
                   [mainWindow, captureOnlyLaunch]() {
                     if (captureOnlyLaunch) {
                       QTimer::singleShot(0, QCoreApplication::instance(),
                                          &QCoreApplication::quit);
                       return;
                     }
                     if (mainWindow && !captureOnlyLaunch) {
                       mainWindow->show();
                       mainWindow->raise();
                       mainWindow->requestActivate();
                     }
                   });

  // Sniper Overlay Window Component
  QQmlComponent sniperComponent(&engine, "ro_screenshot", "SniperOverlay");
  QObject *sniperWindowObject = nullptr;

  QObject::connect(
      &captureEngine, &CaptureEngine::openSniperOverlay, &app,
      [&sniperComponent, &sniperWindowObject](const QString & /*framePath*/,
                                              int frameWidth, int frameHeight) {
        if (!sniperWindowObject) {
          sniperWindowObject = sniperComponent.create();
        }
        if (sniperWindowObject) {
          QQuickWindow *win = qobject_cast<QQuickWindow *>(sniperWindowObject);
          if (win) {
            win->setProperty("sourceFrameWidth", frameWidth);
            win->setProperty("sourceFrameHeight", frameHeight);
            win->showFullScreen();
            win->raise();
            win->requestActivate();
          }
        }
      });

  QObject::connect(&captureEngine, &CaptureEngine::closeSniperOverlay, &app,
                   [&sniperWindowObject]() {
                     if (sniperWindowObject) {
                       QQuickWindow *win =
                           qobject_cast<QQuickWindow *>(sniperWindowObject);
                       if (win) {
                         win->hide();
                       }
                     }
                   });

  // Floating Thumbnail Toast Component
  QQmlComponent toastComponent(&engine, "ro_screenshot", "FloatingThumbnail");
  QObject *toastObject = nullptr;

  QObject::connect(
      &captureEngine, &CaptureEngine::captureSuccess, &app,
      [&toastComponent, &toastObject, &settingsManager, &desktopFeedback](
          const QString &path, const QString &name, bool saved, bool copied) {
        if (settingsManager.showNotification()) {
          desktopFeedback.showCaptureSuccess(path, name, saved, copied);
        }
        if (settingsManager.playShutterSound()) {
          desktopFeedback.playShutter();
        }
        if (settingsManager.showFloatingThumbnail()) {
          if (!toastObject) {
            toastObject = toastComponent.create();
          }
          if (toastObject) {
            QMetaObject::invokeMethod(toastObject, "showToast",
                                      Q_ARG(QVariant, path),
                                      Q_ARG(QVariant, name));
          }
        }
      });

  QObject::connect(
      &captureEngine, &CaptureEngine::captureError, &app,
      [&settingsManager, &desktopFeedback](const QString &message) {
        if (settingsManager.showNotification()) {
          desktopFeedback.showError(message);
        }
        qWarning().noquote() << "[capture]" << message;
      });

  // Connect D-Bus signals to Main Window tabs
  QObject::connect(&dbusService, &DBusService::openLibraryRequested, &app,
                   [mainWindow, rootObject]() {
                     if (mainWindow) {
                       mainWindow->show();
                       mainWindow->raise();
                       mainWindow->requestActivate();
                     }
                     if (rootObject) {
                       rootObject->setProperty("currentTab", 1); // Library tab
                     }
                   });

  QObject::connect(&dbusService, &DBusService::openSettingsRequested, &app,
                   [mainWindow, rootObject]() {
                     if (mainWindow) {
                       mainWindow->show();
                       mainWindow->raise();
                       mainWindow->requestActivate();
                     }
                     if (rootObject) {
                       rootObject->setProperty("currentTab", 2); // Settings tab
                     }
                   });

  QObject::connect(&dbusService, &DBusService::quitRequested, &app,
                   &QGuiApplication::quit);

  // Connect System Tray signals
  QObject::connect(
      &systemTrayManager, &SystemTrayManager::captureRegionRequested,
      &captureEngine,
      [&captureEngine]() { captureEngine.requestRegionCapture(0); });
  QObject::connect(
      &systemTrayManager, &SystemTrayManager::captureFullscreenRequested,
      &captureEngine,
      [&captureEngine]() { captureEngine.requestFullscreenCapture(0); });
  QObject::connect(
      &systemTrayManager, &SystemTrayManager::captureWindowRequested,
      &captureEngine,
      [&captureEngine]() { captureEngine.requestWindowCapture(0); });
  QObject::connect(&systemTrayManager, &SystemTrayManager::openLibraryRequested,
                   &app, [mainWindow, rootObject]() {
                     if (mainWindow) {
                       mainWindow->show();
                       mainWindow->raise();
                     }
                     if (rootObject) {
                       rootObject->setProperty("currentTab", 1);
                     }
                   });
  QObject::connect(&systemTrayManager,
                   &SystemTrayManager::openSettingsRequested, &app,
                   [mainWindow, rootObject]() {
                     if (mainWindow) {
                       mainWindow->show();
                       mainWindow->raise();
                     }
                     if (rootObject) {
                       rootObject->setProperty("currentTab", 2);
                     }
                   });
  QObject::connect(&systemTrayManager, &SystemTrayManager::quitRequested, &app,
                   &QGuiApplication::quit);

  // Handle immediate CLI trigger if requested on launch
  if (parser.isSet(lastRegionOption)) {
    if (mainWindow) {
      mainWindow->hide();
    }
    QTimer::singleShot(100, [&captureEngine, delaySec, captureAction]() {
      captureEngine.requestLastRegionCaptureWithAction(delaySec, captureAction);
    });
  } else if (parser.isSet(regionOption) || parser.isSet(delayOption) ||
             parser.isSet(colorPickerOption)) {
    if (mainWindow) {
      mainWindow->hide();
    }
    QTimer::singleShot(100, [&captureEngine, delaySec, captureAction]() {
      captureEngine.requestRegionCaptureWithAction(delaySec, captureAction);
    });
  } else if (parser.isSet(fullscreenOption)) {
    if (mainWindow) {
      mainWindow->hide();
    }
    QTimer::singleShot(100, [&captureEngine, delaySec, captureAction]() {
      captureEngine.requestFullscreenCaptureWithAction(delaySec, captureAction);
    });
  } else if (parser.isSet(windowOption)) {
    if (mainWindow) {
      mainWindow->hide();
    }
    QTimer::singleShot(100, [&captureEngine, delaySec, captureAction]() {
      captureEngine.requestWindowCaptureWithAction(delaySec, captureAction);
    });
  } else if (parser.isSet(libraryOption)) {
    if (rootObject) {
      rootObject->setProperty("currentTab", 1);
    }
  } else if (parser.isSet(settingsOption)) {
    if (rootObject) {
      rootObject->setProperty("currentTab", 2);
    }
  }

  if (parser.isSet(renderOption)) {
    const QString renderPath = parser.value(renderOption);
    QTimer::singleShot(2000, [mainWindow, &app, renderPath]() {
      if (!mainWindow) {
        app.exit(static_cast<int>(StructuredExitCode::GeneralError));
        return;
      }
      const QImage image = mainWindow->grabWindow();
      const bool saved = image.save(renderPath);
      app.exit(saved ? static_cast<int>(StructuredExitCode::Success)
                     : static_cast<int>(StructuredExitCode::IoError));
    });
    return app.exec();
  }

  return app.exec();
}
