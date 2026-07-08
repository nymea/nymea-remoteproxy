cmake_minimum_required(VERSION 3.19)
project(nymea-remoteproxy-monitor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# FTXUI sourcing, mirroring nymea-cli's CMakeLists.txt: prefer a system package,
# fall back to fetching+building FTXUI ourselves. This is what lets this target
# build on distros (e.g. jammy) where libftxui-dev isn't packaged at all.
set(FTXUI_GIT_TAG "v6.1.9")
option(NYMEA_USE_SYSTEM_FTXUI "Prefer a system-installed FTXUI package over FetchContent." ON)
option(NYMEA_ALLOW_FETCHCONTENT_FTXUI "Allow downloading FTXUI with FetchContent when no system package is available." ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network)

if (NYMEA_USE_SYSTEM_FTXUI)
    find_package(ftxui CONFIG QUIET)
endif()

if (NOT (TARGET ftxui::component AND TARGET ftxui::dom AND TARGET ftxui::screen))
    if (NOT NYMEA_ALLOW_FETCHCONTENT_FTXUI)
        message(FATAL_ERROR "FTXUI was not found as a system package and NYMEA_ALLOW_FETCHCONTENT_FTXUI is OFF.")
    endif()

    include(FetchContent)
    FetchContent_Declare(
        ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
        GIT_TAG ${FTXUI_GIT_TAG}
    )
    set(FTXUI_ENABLE_INSTALL OFF CACHE BOOL "Disable FTXUI install rules when vendored by nymea-remoteproxy-monitor" FORCE)
    FetchContent_MakeAvailable(ftxui)
endif()

if (NOT (TARGET ftxui::component AND TARGET ftxui::dom AND TARGET ftxui::screen))
    message(FATAL_ERROR "FTXUI targets are unavailable. Install a system ftxui package or allow FetchContent to download it.")
endif()

# Keep these three in sync with ../nymea-remoteproxy.pro
set(NYMEA_REMOTEPROXY_MONITOR_SERVER_NAME "nymea-remoteproxy")
set(NYMEA_REMOTEPROXY_MONITOR_API_VERSION "0.6")
set(NYMEA_REMOTEPROXY_MONITOR_COPYRIGHT_YEAR "2023")

# Derive the server version the same way nymea-remoteproxy.pro does, from the
# top-level debian changelog. Falls back to a placeholder so this also
# configures standalone on a machine without dpkg-dev.
set(NYMEA_REMOTEPROXY_MONITOR_SERVER_VERSION "0.0.0-dev")
set(NYMEA_REMOTEPROXY_MONITOR_CHANGELOG "${CMAKE_CURRENT_SOURCE_DIR}/../debian/changelog")
find_program(DPKG_PARSECHANGELOG_EXECUTABLE dpkg-parsechangelog)
if (DPKG_PARSECHANGELOG_EXECUTABLE AND EXISTS "${NYMEA_REMOTEPROXY_MONITOR_CHANGELOG}")
    execute_process(
        COMMAND ${DPKG_PARSECHANGELOG_EXECUTABLE} -l "${NYMEA_REMOTEPROXY_MONITOR_CHANGELOG}"
        COMMAND sed -n -e "s/^Version: //p"
        OUTPUT_VARIABLE NYMEA_REMOTEPROXY_MONITOR_SERVER_VERSION_OUT
        RESULT_VARIABLE NYMEA_REMOTEPROXY_MONITOR_VERSION_RESULT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (NYMEA_REMOTEPROXY_MONITOR_VERSION_RESULT EQUAL 0 AND NOT "${NYMEA_REMOTEPROXY_MONITOR_SERVER_VERSION_OUT}" STREQUAL "")
        set(NYMEA_REMOTEPROXY_MONITOR_SERVER_VERSION "${NYMEA_REMOTEPROXY_MONITOR_SERVER_VERSION_OUT}")
    endif()
endif()

configure_file(version.h.in "${CMAKE_CURRENT_BINARY_DIR}/version.h" @ONLY)

add_executable(nymea-remoteproxy-monitor
    main.cpp
    monitor.h
    monitor.cpp
    monitorclient.h
    monitorclient.cpp
    monitordata.h
    monitordata.cpp
    monitorview.h
    monitorview.cpp
    noninteractivemonitor.h
    noninteractivemonitor.cpp
    utils.h
)

target_include_directories(nymea-remoteproxy-monitor
    PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}"
)

target_link_libraries(nymea-remoteproxy-monitor
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Network
        ftxui::component
        ftxui::dom
        ftxui::screen
)

include(GNUInstallDirs)

install(TARGETS nymea-remoteproxy-monitor
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
