include(../nymea-remoteproxy.pri)
include(../libnymea-remoteproxyclient/libnymea-remoteproxyclient.pri)

TARGET = nymea-tunnelproxy-testutils
TEMPLATE = app

INCLUDEPATH += ../libnymea-remoteproxyclient

LIBS += -L$$top_builddir/libnymea-remoteproxyclient/ -lnymea-remoteproxyclient

SOURCES += main.cpp

target.path = $$[QT_INSTALL_PREFIX]/bin
INSTALLS += target
