cmake_minimum_required(VERSION 3.16)

set(DEFAULT_SERVER_VERSION "0.6.0")
set(SERVER_VERSION_FULL ${DEFAULT_SERVER_VERSION})
if(EXISTS "${CMAKE_SOURCE_DIR}/debian/changelog")
    find_program(DPKG_PARSECHANGELOG_EXECUTABLE dpkg-parsechangelog)
    if(DPKG_PARSECHANGELOG_EXECUTABLE)
        execute_process(
            COMMAND ${DPKG_PARSECHANGELOG_EXECUTABLE} -S Version
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE _dpkg_server_version
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if(_dpkg_server_version)
            set(SERVER_VERSION_FULL ${_dpkg_server_version})
        endif()
    endif()
endif()

set(PROJECT_VERSION_STRING ${SERVER_VERSION_FULL})
if(NOT PROJECT_VERSION_STRING MATCHES "^[0-9]+(\\.[0-9]+){0,3}$")
    string(REGEX MATCH "^[0-9]+(\\.[0-9]+){0,2}" PROJECT_VERSION_STRING "${SERVER_VERSION_FULL}")
    if(NOT PROJECT_VERSION_STRING)
        set(PROJECT_VERSION_STRING ${DEFAULT_SERVER_VERSION})
    endif()
endif()

project(nymea-remoteproxy VERSION ${PROJECT_VERSION_STRING} LANGUAGES CXX)

set(NYMEA_SERVER_VERSION ${SERVER_VERSION_FULL})

include(GNUInstallDirs)

set(SERVER_NAME "nymea-remoteproxy")
set(API_VERSION_MAJOR 0)
set(API_VERSION_MINOR 6)
set(COPYRIGHT_YEAR 2023)

option(NYMEA_ENABLE_TESTS "Build nymea-remoteproxy tests" ON)
option(NYMEA_ENABLE_COVERAGE "Enable coverage instrumentation" OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network WebSockets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network WebSockets)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    set(CMAKE_CXX_STANDARD 17)
else()
    set(CMAKE_CXX_STANDARD 11)
    add_compile_definitions(QT_DISABLE_DEPRECATED_UP_TO=0x050F00)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Werror -g -Wno-deprecated-declarations)
endif()

if(NYMEA_ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(--coverage)
        add_link_options(--coverage)
    endif()
endif()

configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/version.h.in
    ${CMAKE_BINARY_DIR}/version.h
    @ONLY
)

configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/nymea-remoteproxy.pc.in
    ${CMAKE_BINARY_DIR}/nymea-remoteproxy.pc
    @ONLY
)
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/nymea-remoteproxyclient.pc.in
    ${CMAKE_BINARY_DIR}/nymea-remoteproxyclient.pc
    @ONLY
)

install(FILES ${CMAKE_BINARY_DIR}/nymea-remoteproxy.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
install(FILES ${CMAKE_BINARY_DIR}/nymea-remoteproxyclient.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

add_subdirectory(libnymea-remoteproxy)
add_subdirectory(libnymea-remoteproxyclient)
add_subdirectory(server)
add_subdirectory(tunnelclient)
add_subdirectory(monitor)

install(EXPORT nymea-remoteproxyTargets
    NAMESPACE nymea::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nymea-remoteproxy
)

if(NYMEA_ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
