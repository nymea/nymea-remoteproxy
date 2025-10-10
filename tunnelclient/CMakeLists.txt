add_executable(nymea-remoteproxy-tunnelclient
    main.cpp
    clientconnection.cpp
    serverconnection.cpp
)

target_include_directories(nymea-remoteproxy-tunnelclient
    PRIVATE
        ${PROJECT_BINARY_DIR}
)

target_link_libraries(nymea-remoteproxy-tunnelclient
    PRIVATE
        nymea-remoteproxyclient
)

install(TARGETS nymea-remoteproxy-tunnelclient RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
