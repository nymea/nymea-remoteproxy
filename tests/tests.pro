include(../nymea-remoteproxy.pri)

CONFIG += testcase
QT += testlib

TARGET = nymea-remoteproxy-tests

INCLUDEPATH += ../libnymea-remoteproxy
LIBS += -L$$top_builddir/libnymea-remoteproxy/ -lnymea-remoteproxy

RESOURCES += certificate.qrc

HEADERS += nymea-remoteproxy-tests.h
SOURCES += nymea-remoteproxy-tests.cpp

target.path = /usr/bin
INSTALLS += target
