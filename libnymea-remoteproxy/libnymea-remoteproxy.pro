include(../nymea-remoteproxy.pri)

TEMPLATE = lib
TARGET = nymea-remoteproxy

# -L/home/timon/guh/development/cloud/aws-sdk-cpp/build/install/lib
#        -laws-cpp-sdk-access-management \
#        -laws-cpp-sdk-cognito-identity \
#        -laws-cpp-sdk-iam \
#        -laws-cpp-sdk-kinesis\

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
    proxyconfiguration.h \
    tunnelconnection.h \
    authentication/authenticationprocess.h

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
    proxyconfiguration.cpp \
    tunnelconnection.cpp \
    authentication/authenticationprocess.cpp


# install header file with relative subdirectory
for(header, HEADERS) {
    path = /usr/include/nymea-remoteproxy/$${dirname(header)}
    eval(headers_$${path}.files += $${header})
    eval(headers_$${path}.path = $${path})
    eval(INSTALLS *= headers_$${path})
}

target.path = /usr/lib/$$system('dpkg-architecture -q DEB_HOST_MULTIARCH')
INSTALLS += target
