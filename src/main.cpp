#include "core/CaptureEngine.hpp"
#include "core/DBusService.hpp"
#include "core/LanguageManager.hpp"
#include "core/LibraryManager.hpp"
#include "core/SettingsManager.hpp"
#include "core/Types.hpp"
#include "core/UiPreferencesManager.hpp"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDBusInterface>
#include <QDBusReply>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlError>
#include <QQuickWindow>
#include <QTimer>
#include <QTranslator>

using namespace ro_screenshot;

int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);
  app.setApplicationName("ro-screenshot");
  app.setApplicationDisplayName("Ro-ScreenShot");
  app.setOrganizationName("ro-asd");
  app.setOrganizationDomain("ro-asd.org");
  app.setApplicationVersion("1.0.0");

  QCommandLineParser parser;
  parser.setApplicationDescription(
      "Ro-ScreenShot: Modern Linux & Wayland Ekran Yakalama ve Galeri Aracı");
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption regionOption({"r", "region"},
                                  "Bölge seçimi (Sniper) modunda başlat.");
  QCommandLineOption fullscreenOption({"f", "fullscreen"},
                                      "Tam ekran yakalama modunda başlat.");
  QCommandLineOption windowOption({"w", "window"},
                                  "Pencere yakalama modunda başlat.");
  QCommandLineOption libraryOption({"l", "library"},
                                   "Galeri / Kütüphane görünümünü aç.");
  QCommandLineOption settingsOption({"s", "settings"},
                                    "Ayarlar görünümünü aç.");
  QCommandLineOption delayOption({"d", "delay"},
                                 "Yakalama öncesi gecikme süresi (saniye).",
                                 "saniye", "0");
  QCommandLineOption renderOption(
      "render-to", "Main pencereyi PNG olarak çiz ve çık (doc).", "file");

  parser.addOption(regionOption);
  parser.addOption(fullscreenOption);
  parser.addOption(windowOption);
  parser.addOption(libraryOption);
  parser.addOption(settingsOption);
  parser.addOption(delayOption);
  parser.addOption(renderOption);

  parser.process(app);

  int delaySec = parser.value(delayOption).toInt();

  // Check if an instance is already running via D-Bus
  QDBusInterface iface("org.ro_asd.ScreenShot", "/org/ro_asd/ScreenShot",
                       "org.ro_asd.ScreenShot", QDBusConnection::sessionBus());
  if (iface.isValid()) {
    if (parser.isSet(regionOption)) {
      iface.call(QDBus::NoBlock, "CaptureRegion", delaySec);
      return 0;
    } else if (parser.isSet(fullscreenOption)) {
      iface.call(QDBus::NoBlock, "CaptureFullscreen", delaySec);
      return 0;
    } else if (parser.isSet(windowOption)) {
      iface.call(QDBus::NoBlock, "CaptureWindow", delaySec);
      return 0;
    } else if (parser.isSet(settingsOption)) {
      iface.call(QDBus::NoBlock, "OpenSettings");
      return 0;
    } else {
      iface.call(QDBus::NoBlock, "OpenLibrary");
      return 0;
    }
  }

  // First instance initialization
  SettingsManager settingsManager;
  LibraryManager libraryManager(&settingsManager);
  CaptureEngine captureEngine(&settingsManager, &libraryManager);

  DBusService dbusService(&captureEngine);
  dbusService.registerService();

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
    return -1;
  }

  QObject *rootObject = engine.rootObjects().first();
  QQuickWindow *mainWindow = qobject_cast<QQuickWindow *>(rootObject);
  const bool captureOnlyLaunch = parser.isSet(regionOption) ||
                                 parser.isSet(fullscreenOption) ||
                                 parser.isSet(windowOption);

  QObject::connect(&captureEngine, &CaptureEngine::captureUiShouldHide, &app,
                   [mainWindow]() {
                     if (mainWindow) {
                       mainWindow->hide();
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

  QObject::connect(&captureEngine, &CaptureEngine::openSniperOverlay, &app,
                   [&sniperComponent, &sniperWindowObject](
                       const QString & /*framePath*/, int /*w*/, int /*h*/) {
                     if (!sniperWindowObject) {
                       sniperWindowObject = sniperComponent.create();
                     }
                     if (sniperWindowObject) {
                       QQuickWindow *win =
                           qobject_cast<QQuickWindow *>(sniperWindowObject);
                       if (win) {
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

  QObject::connect(&captureEngine, &CaptureEngine::captureSuccess, &app,
                   [&toastComponent, &toastObject,
                    &settingsManager](const QString &path, const QString &name,
                                      bool /*saved*/, bool /*copied*/) {
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

  // Handle immediate CLI trigger if requested on launch
  if (parser.isSet(regionOption)) {
    if (mainWindow) {
      mainWindow->hide();
    }
    QTimer::singleShot(100, [&captureEngine, delaySec]() {
      captureEngine.requestRegionCapture(delaySec);
    });
  } else if (parser.isSet(fullscreenOption)) {
    if (mainWindow) {
      mainWindow->hide();
    }
    QTimer::singleShot(100, [&captureEngine, delaySec]() {
      captureEngine.requestFullscreenCapture(delaySec);
    });
  } else if (parser.isSet(windowOption)) {
    if (mainWindow) {
      mainWindow->hide();
    }
    QTimer::singleShot(100, [&captureEngine, delaySec]() {
      captureEngine.requestWindowCapture(delaySec);
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
        app.exit(1);
        return;
      }
      const QImage image = mainWindow->grabWindow();
      const bool saved = image.save(renderPath);
      app.exit(saved ? 0 : 1);
    });
    return app.exec();
  }

  return app.exec();
}
