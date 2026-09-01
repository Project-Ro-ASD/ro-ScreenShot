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

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Gui
    Quick
    Qml
    DBus
    Widgets
    Test
)

qt_standard_project_setup(REQUIRES 6.5)

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
)

qt_add_executable(ro-screenshot
    src/main.cpp
    ${RO_SCREENSHOT_SOURCES}
)

qt_add_qml_module(ro-screenshot
    URI ro_screenshot
    VERSION 1.0
    RESOURCE_PREFIX "/ro_screenshot"
    QML_FILES
        src/ui/qml/MainHub.qml
        src/ui/qml/LibraryView.qml
        src/ui/qml/SettingsView.qml
        src/ui/qml/SniperOverlay.qml
        src/ui/qml/FloatingThumbnail.qml
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
    Qt6::Widgets
)

# Desktop & AppStream metadata installation
include(GNUInstallDirs)
install(TARGETS ro-screenshot DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES data/org.ro_asd.ro_screenshot.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

# Testing
include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
