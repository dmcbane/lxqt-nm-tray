cmake_minimum_required(VERSION 3.18)

project(lxqt-nm-tray VERSION 0.1.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# lxqt-panel only searches its own install prefix for plugins, so default to /usr.
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Install prefix" FORCE)
endif ()
include(GNUInstallDirs)

find_package(Qt6 6.6 REQUIRED COMPONENTS Widgets Gui DBus)
find_package(KF6NetworkManagerQt 6.5 REQUIRED)

set(NM_TRAY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nm-tray)
if (NOT EXISTS ${NM_TRAY_DIR}/src/nmmodel.cpp)
    message(FATAL_ERROR "nm-tray submodule missing; run: git submodule update --init")
endif ()

# Where lxqt-panel looks for plugin modules and their .desktop descriptors.
set(LXQT_PANEL_PLUGIN_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/lxqt-panel" CACHE PATH "lxqt-panel plugin module dir")
set(LXQT_PANEL_DESKTOP_DIR "${CMAKE_INSTALL_FULL_DATAROOTDIR}/lxqt/lxqt-panel" CACHE PATH "lxqt-panel plugin .desktop dir")
set(NM_TRAY_TRANSLATION_DIR "/usr/share/nm-tray" CACHE PATH "Installed nm-tray translations (reused if present)")

# --- nm-tray core (model, menus, dialogs), built from the pinned submodule ---------------
# main.cpp, tray.cpp and translate.cpp are deliberately excluded: they assume nm-tray owns
# the process (QApplication setup, QSystemTrayIcon, a global Qt translator).
add_library(nmtray_core STATIC
    ${NM_TRAY_DIR}/src/log.cpp
    ${NM_TRAY_DIR}/src/icons.cpp
    ${NM_TRAY_DIR}/src/nmmodel.cpp
    ${NM_TRAY_DIR}/src/nmproxy.cpp
    ${NM_TRAY_DIR}/src/connectioninfo.cpp
    ${NM_TRAY_DIR}/src/nmlist.cpp
    ${NM_TRAY_DIR}/src/menuview.cpp
    ${NM_TRAY_DIR}/src/windowmenu.cpp
    ${NM_TRAY_DIR}/resources/icons.qrc
)
target_include_directories(nmtray_core PUBLIC ${NM_TRAY_DIR}/src)
target_compile_definitions(nmtray_core PUBLIC
    QT_NO_SIGNALS_SLOTS_KEYWORDS
    NM_TRAY_VERSION="0.5.1+escape"
)
target_link_libraries(nmtray_core PUBLIC Qt6::Widgets Qt6::Gui KF6::NetworkManagerQt)

# --- our logic, kept separate from the plugin so it can be unit tested --------------------
add_library(lxqtnmtray_logic STATIC
    src/popupanchor.cpp
    src/nmtraysettings.cpp
    src/statustext.cpp
)
target_include_directories(lxqtnmtray_logic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_definitions(lxqtnmtray_logic PUBLIC QT_NO_SIGNALS_SLOTS_KEYWORDS)
target_link_libraries(lxqtnmtray_logic PUBLIC Qt6::Widgets Qt6::Gui)

# --- the lxqt-panel plugin module: lib<desktop-id>.so --------------------------------------
option(BUILD_PLUGIN "Build the lxqt-panel plugin (needs lxqt-panel headers)" ON)
if (BUILD_PLUGIN)
    find_path(LXQT_PANEL_INCLUDE_DIR lxqt/ilxqtpanelplugin.h REQUIRED)

    qt6_add_dbus_interface(NOTIFY_SRCS
        ${NM_TRAY_DIR}/dbus/org.freedesktop.Notifications.xml
        org_freedesktop_notifications
    )

    add_library(nmtray MODULE
        src/nmtrayplugin.cpp
        src/nmtraystatus.cpp
        ${NOTIFY_SRCS}
    )
    target_include_directories(nmtray PRIVATE ${LXQT_PANEL_INCLUDE_DIR}/lxqt ${CMAKE_CURRENT_BINARY_DIR})
    target_compile_definitions(nmtray PRIVATE
        LXQT_NM_TRAY_VERSION="${PROJECT_VERSION}"
        NM_TRAY_TRANSLATION_DIR="${NM_TRAY_TRANSLATION_DIR}"
    )
    target_link_libraries(nmtray PRIVATE lxqtnmtray_logic nmtray_core Qt6::DBus)
    set_target_properties(nmtray PROPERTIES PREFIX "lib")

    install(TARGETS nmtray DESTINATION ${LXQT_PANEL_PLUGIN_DIR})
    install(FILES resources/nmtray.desktop DESTINATION ${LXQT_PANEL_DESKTOP_DIR})
endif ()

include(CTest)
if (BUILD_TESTING)
    add_subdirectory(tests)
endif ()
