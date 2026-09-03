include(CTest)

set(RO_COMMON_TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/core/Types.hpp
    ${CMAKE_SOURCE_DIR}/src/core/SettingsManager.hpp
    ${CMAKE_SOURCE_DIR}/src/core/SettingsManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/LibraryManager.hpp
    ${CMAKE_SOURCE_DIR}/src/core/LibraryManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/CaptureEngine.hpp
    ${CMAKE_SOURCE_DIR}/src/core/CaptureEngine.cpp
    ${CMAKE_SOURCE_DIR}/src/core/DBusService.hpp
    ${CMAKE_SOURCE_DIR}/src/core/DBusService.cpp
    ${CMAKE_SOURCE_DIR}/src/core/UiPreferencesManager.hpp
    ${CMAKE_SOURCE_DIR}/src/core/UiPreferencesManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/DesktopFeedback.hpp
    ${CMAKE_SOURCE_DIR}/src/core/DesktopFeedback.cpp
    ${CMAKE_SOURCE_DIR}/src/core/platform/IScreenshotProvider.hpp
    ${CMAKE_SOURCE_DIR}/src/core/platform/ScreenGeometryManager.hpp
    ${CMAKE_SOURCE_DIR}/src/core/platform/ScreenGeometryManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/platform/SafeImageWriter.hpp
    ${CMAKE_SOURCE_DIR}/src/core/platform/SafeImageWriter.cpp
    ${CMAKE_SOURCE_DIR}/src/core/platform/WlrScreencopyProvider.hpp
    ${CMAKE_SOURCE_DIR}/src/core/platform/WlrScreencopyProvider.cpp
    ${CMAKE_SOURCE_DIR}/src/core/platform/XdgPortalProvider.hpp
    ${CMAKE_SOURCE_DIR}/src/core/platform/XdgPortalProvider.cpp
    ${CMAKE_SOURCE_DIR}/src/core/platform/X11FallbackProvider.hpp
    ${CMAKE_SOURCE_DIR}/src/core/platform/X11FallbackProvider.cpp
    ${CMAKE_SOURCE_DIR}/src/core/shortcuts/ShortcutManager.hpp
    ${CMAKE_SOURCE_DIR}/src/core/shortcuts/ShortcutManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/OcrEngine.hpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/OcrEngine.cpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/QrCodeDetector.hpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/QrCodeDetector.cpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/ScreenRecorderEngine.hpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/ScreenRecorderEngine.cpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/CloudShareManager.hpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/CloudShareManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/AutostartManager.hpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/AutostartManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/SystemTrayManager.hpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/SystemTrayManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/SensitiveDataDetector.hpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/MockupFrameGenerator.hpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/MockupFrameGenerator.cpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/PaletteExtractor.hpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/PaletteExtractor.cpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/SmartDimensionGuide.hpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/SmartDimensionGuide.cpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/TableExtractor.hpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/TableExtractor.cpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/VaultManager.hpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/VaultManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/EphemeralShotManager.hpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/EphemeralShotManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/MetadataSanitizer.hpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/MetadataSanitizer.cpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/PerceptualHashEngine.hpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/PerceptualHashEngine.cpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/DuplicateCleaner.hpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/DuplicateCleaner.cpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/AccessibilityAnalyzer.hpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/AccessibilityAnalyzer.cpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/PdfReportGenerator.hpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/PdfReportGenerator.cpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/ImageDiffEngine.hpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/ImageDiffEngine.cpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/LiveTranslator.hpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/LiveTranslator.cpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/AiAssistantEngine.hpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/AiAssistantEngine.cpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/ScrollingCaptureEngine.hpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/ScrollingCaptureEngine.cpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/SmartWindowDetector.hpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/SmartWindowDetector.cpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/LivePresentationOverlay.hpp
    ${CMAKE_SOURCE_DIR}/src/core/advanced/LivePresentationOverlay.cpp
)

macro(add_ro_test TEST_NAME TEST_SOURCE)
    add_executable(${TEST_NAME}
        ${TEST_SOURCE}
        ${RO_COMMON_TEST_SOURCES}
    )
    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )
    target_link_libraries(${TEST_NAME} PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::DBus
        Qt6::Multimedia
        Qt6::Test
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endmacro()

add_ro_test(test_settings test_settings.cpp)
add_ro_test(test_library test_library.cpp)
add_ro_test(test_capture test_capture.cpp)
add_ro_test(test_ui_preferences test_ui_preferences.cpp)
add_ro_test(test_multi_monitor_geometry test_multi_monitor_geometry.cpp)
add_ro_test(test_safe_image_io test_safe_image_io.cpp)
add_ro_test(test_portal_backend test_portal_backend.cpp)
add_ro_test(test_shortcuts_and_advanced test_shortcuts_and_advanced.cpp)
add_ro_test(test_cli_dbus test_cli_dbus.cpp)
add_ro_test(test_innovations test_innovations.cpp)
