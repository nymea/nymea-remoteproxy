include(../nymea-remoteproxy.pri)

CONFIG += testcase
QT += testlib

TARGET = nymea-remoteproxy-tests

INCLUDEPATH += ../libnymea-remoteproxy ../libnymea-remoteproxyclient
LIBS += -L$$top_builddir/libnymea-remoteproxy/ -lnymea-remoteproxy \
        -L$$top_builddir/libnymea-remoteproxyclient/ -lnymea-remoteproxyclient \

RESOURCES += certificate.qrc

HEADERS += nymea-remoteproxy-tests.h
SOURCES += nymea-remoteproxy-tests.cpp

target.path = /usr/bin
INSTALLS += target
