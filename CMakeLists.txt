cmake_minimum_required(VERSION 3.22)

project(ro-screenshot
    VERSION 1.0.0
    DESCRIPTION "Modern Linux & Wayland Screen Capture & Gallery Tool"
    HOMEPAGE_URL "https://github.com/Project-Ro-ASD/Ro-ScreenShot"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for language servers
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler Warnings and Security Flags
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wformat=2
        -Wconversion
        -Wsign-conversion
        -Wshadow
        -Wnon-virtual-dtor
    )
endif()

# Sanitizer Options
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)

if(ENABLE_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()
if(ENABLE_UBSAN)
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=undefined)
endif()
if(ENABLE_TSAN)
    add_compile_options(-fsanitize=thread)
    add_link_options(-fsanitize=thread)
endif()

include(CTest)

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Gui
    Quick
    Qml
    DBus
    Multimedia
)

if(BUILD_TESTING)
    find_package(Qt6 6.5 REQUIRED COMPONENTS Test)
endif()

qt_standard_project_setup(REQUIRES 6.5)
qt_policy(SET QTP0004 NEW)

set(RO_SCREENSHOT_SOURCES
    src/core/Types.hpp
    src/core/SettingsManager.hpp
    src/core/SettingsManager.cpp
    src/core/LibraryManager.hpp
    src/core/LibraryManager.cpp
    src/core/CaptureEngine.hpp
    src/core/CaptureEngine.cpp
    src/core/DBusService.hpp
    src/core/DBusService.cpp
    src/core/LanguageManager.hpp
    src/core/LanguageManager.cpp
    src/core/UiPreferencesManager.hpp
    src/core/UiPreferencesManager.cpp
    src/core/DesktopFeedback.hpp
    src/core/DesktopFeedback.cpp
    src/core/platform/IScreenshotProvider.hpp
    src/core/platform/ScreenGeometryManager.hpp
    src/core/platform/ScreenGeometryManager.cpp
    src/core/platform/SafeImageWriter.hpp
    src/core/platform/SafeImageWriter.cpp
    src/core/platform/WlrScreencopyProvider.hpp
    src/core/platform/WlrScreencopyProvider.cpp
    src/core/platform/XdgPortalProvider.hpp
    src/core/platform/XdgPortalProvider.cpp
    src/core/platform/X11FallbackProvider.hpp
    src/core/platform/X11FallbackProvider.cpp
    src/core/shortcuts/ShortcutManager.hpp
    src/core/shortcuts/ShortcutManager.cpp
    src/core/advanced/OcrEngine.hpp
    src/core/advanced/OcrEngine.cpp
    src/core/advanced/QrCodeDetector.hpp
    src/core/advanced/QrCodeDetector.cpp
    src/core/advanced/ScreenRecorderEngine.hpp
    src/core/advanced/ScreenRecorderEngine.cpp
    src/core/advanced/CloudShareManager.hpp
    src/core/advanced/CloudShareManager.cpp
    src/core/advanced/AutostartManager.hpp
    src/core/advanced/AutostartManager.cpp
    src/core/advanced/SystemTrayManager.hpp
    src/core/advanced/SystemTrayManager.cpp
    src/core/advanced/SensitiveDataDetector.hpp
    src/core/advanced/MockupFrameGenerator.hpp
    src/core/advanced/MockupFrameGenerator.cpp
    src/core/advanced/PaletteExtractor.hpp
    src/core/advanced/PaletteExtractor.cpp
    src/core/advanced/SmartDimensionGuide.hpp
    src/core/advanced/SmartDimensionGuide.cpp
    src/core/advanced/TableExtractor.hpp
    src/core/advanced/TableExtractor.cpp
    src/core/advanced/VaultManager.hpp
    src/core/advanced/VaultManager.cpp
    src/core/advanced/EphemeralShotManager.hpp
    src/core/advanced/EphemeralShotManager.cpp
    src/core/advanced/MetadataSanitizer.hpp
    src/core/advanced/MetadataSanitizer.cpp
    src/core/advanced/PerceptualHashEngine.hpp
    src/core/advanced/PerceptualHashEngine.cpp
    src/core/advanced/DuplicateCleaner.hpp
    src/core/advanced/DuplicateCleaner.cpp
    src/core/advanced/AccessibilityAnalyzer.hpp
    src/core/advanced/AccessibilityAnalyzer.cpp
    src/core/advanced/PdfReportGenerator.hpp
    src/core/advanced/PdfReportGenerator.cpp
    src/core/advanced/ImageDiffEngine.hpp
    src/core/advanced/ImageDiffEngine.cpp
    src/core/advanced/LiveTranslator.hpp
    src/core/advanced/LiveTranslator.cpp
    src/core/advanced/AiAssistantEngine.hpp
    src/core/advanced/AiAssistantEngine.cpp
    src/core/advanced/ScrollingCaptureEngine.hpp
    src/core/advanced/ScrollingCaptureEngine.cpp
    src/core/advanced/SmartWindowDetector.hpp
    src/core/advanced/SmartWindowDetector.cpp
    src/core/advanced/LivePresentationOverlay.hpp
    src/core/advanced/LivePresentationOverlay.cpp
)

qt_add_executable(ro-screenshot
    src/main.cpp
    ${RO_SCREENSHOT_SOURCES}
)

set_source_files_properties(src/ui/qml/MainHub.qml PROPERTIES QT_RESOURCE_ALIAS "MainHub.qml")
set_source_files_properties(src/ui/qml/LibraryView.qml PROPERTIES QT_RESOURCE_ALIAS "LibraryView.qml")
set_source_files_properties(src/ui/qml/SettingsView.qml PROPERTIES QT_RESOURCE_ALIAS "SettingsView.qml")
set_source_files_properties(src/ui/qml/SniperOverlay.qml PROPERTIES QT_RESOURCE_ALIAS "SniperOverlay.qml")
set_source_files_properties(src/ui/qml/FloatingThumbnail.qml PROPERTIES QT_RESOURCE_ALIAS "FloatingThumbnail.qml")
set_source_files_properties(src/ui/qml/FloatingRecordingHUD.qml PROPERTIES QT_RESOURCE_ALIAS "FloatingRecordingHUD.qml")
set_source_files_properties(src/ui/qml/LivePresentationOverlay.qml PROPERTIES QT_RESOURCE_ALIAS "LivePresentationOverlay.qml")
set_source_files_properties(src/ui/qml/components/AccessibleIconButton.qml PROPERTIES QT_RESOURCE_ALIAS "components/AccessibleIconButton.qml")
set_source_files_properties(src/ui/qml/components/AppSnackbar.qml PROPERTIES QT_RESOURCE_ALIAS "components/AppSnackbar.qml")
set_source_files_properties(src/ui/qml/components/BusyOverlay.qml PROPERTIES QT_RESOURCE_ALIAS "components/BusyOverlay.qml")
set_source_files_properties(src/ui/qml/components/ConfirmDialog.qml PROPERTIES QT_RESOURCE_ALIAS "components/ConfirmDialog.qml")
set_source_files_properties(src/ui/qml/components/InlineStatus.qml PROPERTIES QT_RESOURCE_ALIAS "components/InlineStatus.qml")
set_source_files_properties(src/ui/qml/components/ImageDiffDialog.qml PROPERTIES QT_RESOURCE_ALIAS "components/ImageDiffDialog.qml")
set_source_files_properties(src/ui/qml/components/PdfExportDialog.qml PROPERTIES QT_RESOURCE_ALIAS "components/PdfExportDialog.qml")
set_source_files_properties(src/ui/qml/components/MockupFrameDialog.qml PROPERTIES QT_RESOURCE_ALIAS "components/MockupFrameDialog.qml")
set_source_files_properties(src/ui/qml/components/DevKitDialog.qml PROPERTIES QT_RESOURCE_ALIAS "components/DevKitDialog.qml")
set_source_files_properties(src/ui/qml/components/VaultDialog.qml PROPERTIES QT_RESOURCE_ALIAS "components/VaultDialog.qml")
set_source_files_properties(src/ui/qml/editor/AnnotationEditor.qml PROPERTIES QT_RESOURCE_ALIAS "editor/AnnotationEditor.qml")
set_source_files_properties(src/ui/qml/editor/EditorCanvas.qml PROPERTIES QT_RESOURCE_ALIAS "editor/EditorCanvas.qml")
set_source_files_properties(src/ui/qml/editor/EditorToolbar.qml PROPERTIES QT_RESOURCE_ALIAS "editor/EditorToolbar.qml")

# Flatten SVG asset paths
set(RO_SVG_ASSETS
    src/ui/assets/logo.svg
    src/ui/assets/icon-capture.svg
    src/ui/assets/icon-gallery.svg
    src/ui/assets/icon-settings.svg
    src/ui/assets/icon-language.svg
    src/ui/assets/icon-fullscreen.svg
    src/ui/assets/icon-window.svg
    src/ui/assets/icon-timer.svg
    src/ui/assets/icon-copy.svg
    src/ui/assets/icon-folder.svg
    src/ui/assets/icon-trash.svg
    src/ui/assets/icon-search.svg
    src/ui/assets/icon-refresh.svg
    src/ui/assets/icon-gallery-empty.svg
    src/ui/assets/icon-tags.svg
    src/ui/assets/icon-bolt.svg
    src/ui/assets/icon-crosshair.svg
    src/ui/assets/icon-theme.svg
)
foreach(_svg ${RO_SVG_ASSETS})
    get_filename_component(_basename "${_svg}" NAME)
    set_source_files_properties(${_svg} PROPERTIES QT_RESOURCE_ALIAS "assets/${_basename}")
endforeach()

qt_add_qml_module(ro-screenshot
    URI "ro_screenshot"
    VERSION 1.0
    QML_FILES
        src/ui/qml/MainHub.qml
        src/ui/qml/LibraryView.qml
        src/ui/qml/SettingsView.qml
        src/ui/qml/SniperOverlay.qml
        src/ui/qml/FloatingThumbnail.qml
        src/ui/qml/FloatingRecordingHUD.qml
        src/ui/qml/LivePresentationOverlay.qml
        src/ui/qml/components/AccessibleIconButton.qml
        src/ui/qml/components/AppSnackbar.qml
        src/ui/qml/components/BusyOverlay.qml
        src/ui/qml/components/ConfirmDialog.qml
        src/ui/qml/components/InlineStatus.qml
        src/ui/qml/components/ImageDiffDialog.qml
        src/ui/qml/components/PdfExportDialog.qml
        src/ui/qml/components/MockupFrameDialog.qml
        src/ui/qml/components/DevKitDialog.qml
        src/ui/qml/components/VaultDialog.qml
        src/ui/qml/editor/AnnotationEditor.qml
        src/ui/qml/editor/EditorCanvas.qml
        src/ui/qml/editor/EditorToolbar.qml
    RESOURCES
        ${RO_SVG_ASSETS}
)

# Embed compiled translations as Qt resources
qt_add_resources(ro-screenshot "i18n"
    PREFIX "/i18n"
    BASE "i18n"
    FILES
        i18n/ro-screenshot_en.qm
        i18n/ro-screenshot_tr.qm
        i18n/ro-screenshot_de.qm
        i18n/ro-screenshot_es.qm
)

target_include_directories(ro-screenshot PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(ro-screenshot PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Qml
    Qt6::DBus
    Qt6::Multimedia
)

# Desktop & AppStream metadata installation
include(GNUInstallDirs)
install(TARGETS ro-screenshot DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES data/org.ro_asd.ro_screenshot.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

# Testing
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
