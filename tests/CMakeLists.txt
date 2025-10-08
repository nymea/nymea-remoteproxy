find_package(Qt5 REQUIRED COMPONENTS Test)

set(TEST_BASE_SOURCES
    testbase/basetest.cpp
)

set(TEST_BASE_HEADERS
    testbase/basetest.h
)

add_library(remoteproxy-testbase STATIC
    ${TEST_BASE_SOURCES}
    ${TEST_BASE_HEADERS}
)

target_include_directories(remoteproxy-testbase
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/testbase
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/libnymea-remoteproxy
        ${PROJECT_SOURCE_DIR}/libnymea-remoteproxyclient
        ${PROJECT_SOURCE_DIR}/common
        ${PROJECT_BINARY_DIR}
)

target_link_libraries(remoteproxy-testbase
    PUBLIC
        Qt5::Test
        Qt5::Core
        Qt5::Network
        Qt5::WebSockets
        nymea-remoteproxy-lib
        nymea-remoteproxyclient
)

add_executable(test-tunnelproxy
    test-tunnelproxy/remoteproxyteststunnelproxy.cpp
    test-tunnelproxy/remoteproxyteststunnelproxy.h
    resources/resources.qrc
)

target_link_libraries(test-tunnelproxy
    PRIVATE
        remoteproxy-testbase
)

target_include_directories(test-tunnelproxy
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/test-tunnelproxy
)

add_test(NAME tunnelproxy COMMAND test-tunnelproxy)
