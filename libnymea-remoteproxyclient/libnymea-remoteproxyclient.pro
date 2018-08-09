include(../nymea-remoteproxy.pri)

TEMPLATE = lib
TARGET = nymea-remoteproxyclient

HEADERS += \
    jsonrpcclient.h \
    jsonreply.h \
    remoteproxyconnection.h \
    proxyconnection.h \
    websocketconnection.h


SOURCES += \
    jsonrpcclient.cpp \
    jsonreply.cpp \
    remoteproxyconnection.cpp \
    proxyconnection.cpp \
    websocketconnection.cpp


# install header file with relative subdirectory
for(header, HEADERS) {
    path = /usr/include/nymea-remoteproxyclient/$${dirname(header)}
    eval(headers_$${path}.files += $${header})
    eval(headers_$${path}.path = $${path})
    eval(INSTALLS *= headers_$${path})
}
