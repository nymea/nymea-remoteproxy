include(../nymea-remoteproxy.pri)

TEMPLATE = lib
TARGET = nymea-remoteproxy

HEADERS += \
    engine.h \
    loggingcategories.h \
    transportinterface.h \
    websocketserver.h \
    proxyclient.h \
    proxyserver.h \
    jsonrpcserver.h \
    jsonrpc/jsonhandler.h \
    jsonrpc/jsonreply.h \
    jsonrpc/jsontypes.h \
    jsonrpc/authenticationhandler.h \
    authentication/authenticator.h \
    authentication/awsauthenticator.h \
    authentication/authenticationreply.h \
    proxyconfiguration.h

SOURCES += \
    engine.cpp \
    loggingcategories.cpp \
    transportinterface.cpp \
    websocketserver.cpp \
    proxyclient.cpp \
    proxyserver.cpp \
    jsonrpcserver.cpp \
    jsonrpc/jsonhandler.cpp \
    jsonrpc/jsonreply.cpp \
    jsonrpc/jsontypes.cpp \
    jsonrpc/authenticationhandler.cpp \
    authentication/authenticator.cpp \
    authentication/awsauthenticator.cpp \
    authentication/authenticationreply.cpp \
    proxyconfiguration.cpp


# install header file with relative subdirectory
for(header, HEADERS) {
    path = /usr/include/nymea-remoteproxy/$${dirname(header)}
    eval(headers_$${path}.files += $${header})
    eval(headers_$${path}.path = $${path})
    eval(INSTALLS *= headers_$${path})
}
