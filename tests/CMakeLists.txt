include(CTest)

macro(add_ro_test TEST_NAME TEST_SOURCE)
    add_executable(${TEST_NAME}
        ${TEST_SOURCE}
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
    )
    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )
    target_link_libraries(${TEST_NAME} PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::DBus
        Qt6::Test
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endmacro()

add_ro_test(test_settings test_settings.cpp)
add_ro_test(test_library test_library.cpp)
add_ro_test(test_capture test_capture.cpp)
add_ro_test(test_ui_preferences test_ui_preferences.cpp)
