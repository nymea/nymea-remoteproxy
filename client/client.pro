include(../nymea-remoteproxy.pri)
include(../libnymea-remoteproxyclient/libnymea-remoteproxyclient.pri)

TARGET = nymea-remoteproxy-client
TEMPLATE = app

INCLUDEPATH += ../libnymea-remoteproxy

LIBS += -L$$top_builddir/libnymea-remoteproxyclient/ -lnymea-remoteproxyclient

SOURCES += main.cpp \
    proxyclient.cpp

target.path = $$[QT_INSTALL_PREFIX]/bin
INSTALLS += target

HEADERS += \
    proxyclient.h
