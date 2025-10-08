set(LIBNYMEA_REMOTE_PROXY_SOURCES
    engine.cpp
    logengine.cpp
    loggingcategories.cpp
    proxyconfiguration.cpp
    jsonrpc/jsonhandler.cpp
    jsonrpc/jsonreply.cpp
    jsonrpc/jsontypes.cpp
    jsonrpc/tunnelproxyhandler.cpp
    server/tcpsocketserver.cpp
    server/transportinterface.cpp
    server/transportclient.cpp
    server/unixsocketserver.cpp
    server/websocketserver.cpp
    server/jsonrpcserver.cpp
    server/monitorserver.cpp
    tunnelproxy/tunnelproxyclient.cpp
    tunnelproxy/tunnelproxyclientconnection.cpp
    tunnelproxy/tunnelproxyserver.cpp
    tunnelproxy/tunnelproxyserverconnection.cpp
    ../common/slipdataprocessor.cpp
)

set(LIBNYMEA_REMOTE_PROXY_HEADERS
    engine.h
    logengine.h
    loggingcategories.h
    proxyconfiguration.h
    jsonrpc/jsonhandler.h
    jsonrpc/jsonreply.h
    jsonrpc/jsontypes.h
    jsonrpc/tunnelproxyhandler.h
    server/tcpsocketserver.h
    server/transportinterface.h
    server/unixsocketserver.h
    server/websocketserver.h
    server/jsonrpcserver.h
    server/transportclient.h
    server/monitorserver.h
    tunnelproxy/tunnelproxyclient.h
    tunnelproxy/tunnelproxyclientconnection.h
    tunnelproxy/tunnelproxyserver.h
    tunnelproxy/tunnelproxyserverconnection.h
    ../common/slipdataprocessor.h
)

add_library(nymea-remoteproxy-lib SHARED
    ${LIBNYMEA_REMOTE_PROXY_SOURCES}
)

set_target_properties(nymea-remoteproxy-lib PROPERTIES
    VERSION ${NYMEA_LIBRARY_VERSION}
    SOVERSION ${NYMEA_LIBRARY_VERSION}
    EXPORT_NAME nymea-remoteproxy
    OUTPUT_NAME nymea-remoteproxy
)

target_include_directories(nymea-remoteproxy-lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/common>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/nymea-remoteproxy>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/nymea-remoteproxy/common>
)

target_link_libraries(nymea-remoteproxy-lib
    PUBLIC
        Qt6::Core
        Qt6::Network
        Qt6::WebSockets
)

install(TARGETS nymea-remoteproxy-lib
    EXPORT nymea-remoteproxyTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

foreach(header IN LISTS LIBNYMEA_REMOTE_PROXY_HEADERS)
    get_filename_component(header_dir "${header}" DIRECTORY)
    if(header_dir)
        string(REGEX REPLACE "^\.\./" "" header_dir "${header_dir}")
        set(install_dir "${CMAKE_INSTALL_INCLUDEDIR}/nymea-remoteproxy/${header_dir}")
    else()
        set(install_dir "${CMAKE_INSTALL_INCLUDEDIR}/nymea-remoteproxy")
    endif()
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${header} DESTINATION ${install_dir})
endforeach()

install(FILES ${PROJECT_BINARY_DIR}/version.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nymea-remoteproxy)
