include(../nymea-remoteproxy.pri)
include(../common/common.pri)

TEMPLATE = lib
TARGET = nymea-remoteproxy

HEADERS += \
    engine.h \
    logengine.h \
    loggingcategories.h \
    proxyconfiguration.h \
    jsonrpc/jsonhandler.h \
    jsonrpc/jsonreply.h \
    jsonrpc/jsontypes.h \
    jsonrpc/tunnelproxyhandler.h \
    server/tcpsocketserver.h \
    server/transportinterface.h \
    server/unixsocketserver.h \
    server/websocketserver.h \
    server/jsonrpcserver.h \
    server/transportclient.h \
    server/monitorserver.h \
    tunnelproxy/tunnelproxyclient.h \
    tunnelproxy/tunnelproxyclientconnection.h \
    tunnelproxy/tunnelproxyserver.h \
    tunnelproxy/tunnelproxyserverconnection.h

SOURCES += \
    engine.cpp \
    logengine.cpp \
    loggingcategories.cpp \
    proxyconfiguration.cpp \
    jsonrpc/jsonhandler.cpp \
    jsonrpc/jsonreply.cpp \
    jsonrpc/jsontypes.cpp \
    jsonrpc/tunnelproxyhandler.cpp \
    server/tcpsocketserver.cpp \
    server/transportinterface.cpp \
    server/transportclient.cpp \
    server/unixsocketserver.cpp \
    server/websocketserver.cpp \
    server/jsonrpcserver.cpp \
    server/monitorserver.cpp \
    tunnelproxy/tunnelproxyclient.cpp \
    tunnelproxy/tunnelproxyclientconnection.cpp \
    tunnelproxy/tunnelproxyserver.cpp \
    tunnelproxy/tunnelproxyserverconnection.cpp


# install header file with relative subdirectory
for (header, HEADERS) {
    path = $$[QT_INSTALL_PREFIX]/include/nymea-remoteproxy/$${dirname(header)}
    eval(headers_$${path}.files += $${header})
    eval(headers_$${path}.path = $${path})
    eval(INSTALLS *= headers_$${path})
}

target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target

CONFIG += create_pc create_prl no_install_prl
QMAKE_PKGCONFIG_NAME = libnymea-remoteproxy
QMAKE_PKGCONFIG_DESCRIPTION = nymea remoteproxy development library
QMAKE_PKGCONFIG_PREFIX = $$[QT_INSTALL_PREFIX]
QMAKE_PKGCONFIG_INCDIR = $$[QT_INSTALL_PREFIX]/include/nymea-remoteproxy/
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_VERSION = $$SERVER_VERSION
QMAKE_PKGCONFIG_FILE = nymea-remoteproxy
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
