include(${PROJECT_SOURCE_DIR}/cmake/nymea-remoteproxyclient-sources.cmake)

add_library(nymea-remoteproxyclient SHARED
    ${NYMEA_REMOTEPROXYCLIENT_SOURCES}
)

target_sources(nymea-remoteproxyclient PRIVATE
    ${NYMEA_REMOTEPROXYCLIENT_HEADERS}
)

set_target_properties(nymea-remoteproxyclient PROPERTIES
    VERSION ${NYMEA_LIBRARY_VERSION}
    SOVERSION ${NYMEA_LIBRARY_VERSION_MAJOR}
    EXPORT_NAME nymea-remoteproxyclient
)

nymea_add_library_version_links(nymea-remoteproxyclient)

target_include_directories(nymea-remoteproxyclient
    PUBLIC
        $<BUILD_INTERFACE:${NYMEA_REMOTEPROXYCLIENT_SOURCE_ROOT}>
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

foreach(header IN LISTS NYMEA_REMOTEPROXYCLIENT_HEADER_RELATIVE_PATHS)
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

nymea_install_pkgconfig_for_library(nymea-remoteproxyclient
    BASENAME nymea-remoteproxyclient
    DESCRIPTION "nymea remoteproxy client development library"
)
