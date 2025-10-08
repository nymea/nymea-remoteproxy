set(LIBNYMEA_REMOTE_PROXYCLIENT_SOURCES
    tunnelproxy/tunnelproxyremoteconnection.cpp
    tunnelproxy/tunnelproxysocket.cpp
    tunnelproxy/tunnelproxysocketserver.cpp
    tcpsocketconnection.cpp
    proxyjsonrpcclient.cpp
    jsonreply.cpp
    proxyconnection.cpp
    websocketconnection.cpp
    ../common/slipdataprocessor.cpp
)

set(LIBNYMEA_REMOTE_PROXYCLIENT_HEADERS
    tunnelproxy/tunnelproxyremoteconnection.h
    tunnelproxy/tunnelproxysocket.h
    tunnelproxy/tunnelproxysocketserver.h
    tcpsocketconnection.h
    proxyjsonrpcclient.h
    jsonreply.h
    proxyconnection.h
    websocketconnection.h
    ../common/slipdataprocessor.h
)

add_library(nymea-remoteproxyclient SHARED
    ${LIBNYMEA_REMOTE_PROXYCLIENT_SOURCES}
)

set_target_properties(nymea-remoteproxyclient PROPERTIES
    VERSION ${NYMEA_LIBRARY_VERSION}
    SOVERSION ${NYMEA_LIBRARY_VERSION}
    EXPORT_NAME nymea-remoteproxyclient
)

target_include_directories(nymea-remoteproxyclient
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/common>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/nymea-remoteproxyclient>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/nymea-remoteproxyclient/common>
)

target_link_libraries(nymea-remoteproxyclient
    PUBLIC
        Qt6::Core
        Qt6::Network
        Qt6::WebSockets
)

install(TARGETS nymea-remoteproxyclient
    EXPORT nymea-remoteproxyTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

foreach(header IN LISTS LIBNYMEA_REMOTE_PROXYCLIENT_HEADERS)
    get_filename_component(header_dir "${header}" DIRECTORY)
    if(header_dir)
        string(REGEX REPLACE "^\.\./" "" header_dir "${header_dir}")
        set(install_dir "${CMAKE_INSTALL_INCLUDEDIR}/nymea-remoteproxyclient/${header_dir}")
    else()
        set(install_dir "${CMAKE_INSTALL_INCLUDEDIR}/nymea-remoteproxyclient")
    endif()
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${header} DESTINATION ${install_dir})
endforeach()

install(FILES ${PROJECT_BINARY_DIR}/version.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nymea-remoteproxyclient)
