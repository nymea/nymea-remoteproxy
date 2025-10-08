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

find_package(Qt6 REQUIRED COMPONENTS Core Network WebSockets)

set(CMAKE_CXX_STANDARD 17)
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
    ${CMAKE_SOURCE_DIR}/version.h.in
    ${CMAKE_BINARY_DIR}/version.h
    @ONLY
)

function(nymea_configure_pkgconfig output_basename pkg_name include_subdir library_name description)
    set(_pkgconfig_template [=[
prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
libdir=@CMAKE_INSTALL_FULL_LIBDIR@
includedir=@CMAKE_INSTALL_FULL_INCLUDEDIR@/@INCLUDE_SUBDIR@

Name: @PACKAGE_NAME@
Description: @PACKAGE_DESCRIPTION@
Version: @NYMEA_SERVER_VERSION@
Libs: -L${libdir} -l@LIBRARY_NAME@
Cflags: -I${includedir}
]=])

    set(PACKAGE_NAME "${pkg_name}")
    set(PACKAGE_DESCRIPTION "${description}")
    set(INCLUDE_SUBDIR "${include_subdir}")
    set(LIBRARY_NAME "${library_name}")

    string(CONFIGURE "${_pkgconfig_template}" _pkgconfig_content @ONLY)

    set(_output "${CMAKE_BINARY_DIR}/${output_basename}.pc")
    file(WRITE "${_output}" "${_pkgconfig_content}")

    install(FILES "${_output}" DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endfunction()

nymea_configure_pkgconfig(
    nymea-remoteproxy
    "libnymea-remoteproxy"
    "nymea-remoteproxy"
    "nymea-remoteproxy"
    "nymea remoteproxy development library"
)
nymea_configure_pkgconfig(
    nymea-remoteproxyclient
    "libnymea-remoteproxyclient"
    "nymea-remoteproxyclient"
    "nymea-remoteproxyclient"
    "nymea remoteproxy client development library"
)

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
