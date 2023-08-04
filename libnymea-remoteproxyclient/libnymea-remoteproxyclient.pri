include(../common/common.pri)

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/tunnelproxy/tunnelproxyremoteconnection.h \
    $$PWD/tunnelproxy/tunnelproxysocket.h \
    $$PWD/tunnelproxy/tunnelproxysocketserver.h \
    $$PWD/tcpsocketconnection.h \
    $$PWD/proxyjsonrpcclient.h \
    $$PWD/jsonreply.h \
    $$PWD/proxyconnection.h \
    $$PWD/websocketconnection.h

SOURCES += \
    $$PWD/tunnelproxy/tunnelproxyremoteconnection.cpp \
    $$PWD/tunnelproxy/tunnelproxysocket.cpp \
    $$PWD/tunnelproxy/tunnelproxysocketserver.cpp \
    $$PWD/tcpsocketconnection.cpp \
    $$PWD/proxyjsonrpcclient.cpp \
    $$PWD/jsonreply.cpp \
    $$PWD/proxyconnection.cpp \
    $$PWD/websocketconnection.cpp

