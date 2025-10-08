find_package(Curses REQUIRED)

add_executable(nymea-remoteproxy-monitor
    main.cpp
    monitorclient.cpp
    noninteractivemonitor.cpp
    terminalwindow.cpp
    monitor.cpp
)

target_include_directories(nymea-remoteproxy-monitor
    PRIVATE
        ${PROJECT_BINARY_DIR}
        ${CURSES_INCLUDE_DIRS}
)

target_link_libraries(nymea-remoteproxy-monitor
    PRIVATE
        Qt6::Core
        Qt6::Network
        Qt6::WebSockets
        ${CURSES_LIBRARIES}
)

install(TARGETS nymea-remoteproxy-monitor RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
