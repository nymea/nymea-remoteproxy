add_executable(nymea-remoteproxy
    main.cpp
    remoteproxyserverapplication.cpp
)

target_include_directories(nymea-remoteproxy
    PRIVATE
        ${PROJECT_BINARY_DIR}
)

target_link_libraries(nymea-remoteproxy
    PRIVATE
        nymea-remoteproxy-lib
)

install(TARGETS nymea-remoteproxy RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
