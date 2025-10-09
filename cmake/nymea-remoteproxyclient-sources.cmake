if(DEFINED NYMEA_REMOTEPROXYCLIENT_SOURCES_INCLUDED)
    return()
endif()
set(NYMEA_REMOTEPROXYCLIENT_SOURCES_INCLUDED TRUE)

get_filename_component(_nymea_remoteproxyclient_root "${CMAKE_CURRENT_LIST_DIR}/../libnymea-remoteproxyclient" ABSOLUTE)
get_filename_component(_nymea_remoteproxyclient_common_root "${CMAKE_CURRENT_LIST_DIR}/../common" ABSOLUTE)

set(NYMEA_REMOTEPROXYCLIENT_SOURCE_ROOT "${_nymea_remoteproxyclient_root}")
set(NYMEA_REMOTEPROXYCLIENT_COMMON_ROOT "${_nymea_remoteproxyclient_common_root}")

set(_nymea_remoteproxyclient_source_relative_paths
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

set(_nymea_remoteproxyclient_header_relative_paths
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

set(NYMEA_REMOTEPROXYCLIENT_SOURCES)
foreach(_nymea_source IN LISTS _nymea_remoteproxyclient_source_relative_paths)
    if(_nymea_source MATCHES "^../common/")
        string(REGEX REPLACE "^\.\./common/" "" _nymea_common_relative "${_nymea_source}")
        list(APPEND NYMEA_REMOTEPROXYCLIENT_SOURCES "${_nymea_remoteproxyclient_common_root}/${_nymea_common_relative}")
    else()
        list(APPEND NYMEA_REMOTEPROXYCLIENT_SOURCES "${_nymea_remoteproxyclient_root}/${_nymea_source}")
    endif()
endforeach()

set(NYMEA_REMOTEPROXYCLIENT_HEADERS)
foreach(_nymea_header IN LISTS _nymea_remoteproxyclient_header_relative_paths)
    if(_nymea_header MATCHES "^../common/")
        string(REGEX REPLACE "^\.\./common/" "" _nymea_common_relative "${_nymea_header}")
        list(APPEND NYMEA_REMOTEPROXYCLIENT_HEADERS "${_nymea_remoteproxyclient_common_root}/${_nymea_common_relative}")
    else()
        list(APPEND NYMEA_REMOTEPROXYCLIENT_HEADERS "${_nymea_remoteproxyclient_root}/${_nymea_header}")
    endif()
endforeach()

set(NYMEA_REMOTEPROXYCLIENT_HEADER_RELATIVE_PATHS ${_nymea_remoteproxyclient_header_relative_paths})

unset(_nymea_remoteproxyclient_source_relative_paths)
unset(_nymea_remoteproxyclient_header_relative_paths)

