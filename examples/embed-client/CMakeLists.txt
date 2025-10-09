cmake_minimum_required(VERSION 3.16)
project(nymea_remoteproxyclient_embed_example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Adjust this path if you move the example directory.
set(NYMEA_REMOTE_PROXY_DIR "${CMAKE_CURRENT_LIST_DIR}/../..")

find_package(Qt6 REQUIRED COMPONENTS Core Network WebSockets)

include("${NYMEA_REMOTE_PROXY_DIR}/cmake/nymea-remoteproxyclient-sources.cmake")

add_library(nymea-remoteproxyclient STATIC
    ${NYMEA_REMOTEPROXYCLIENT_SOURCES}
)

target_link_libraries(nymea-remoteproxyclient
    PUBLIC
        Qt6::Core
        Qt6::Network
        Qt6::WebSockets
)

target_include_directories(nymea-remoteproxyclient
    PUBLIC
        ${NYMEA_REMOTEPROXYCLIENT_SOURCE_ROOT}
)

configure_file(
    "${NYMEA_REMOTE_PROXY_DIR}/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/nymea-remoteproxy/version.h"
    @ONLY
)

# Propagate the generated version header to dependents.
target_include_directories(nymea-remoteproxyclient
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/nymea-remoteproxy>
)

add_executable(client-sample
    main.cpp
)

target_link_libraries(client-sample
    PRIVATE
        nymea-remoteproxyclient
)
