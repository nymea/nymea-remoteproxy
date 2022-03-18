include(../../nymea-remoteproxy.pri)
include(../testbase/testbase.pri)

CONFIG += testcase
QT += testlib

TARGET = nymea-remoteproxy-proxy-tests

HEADERS += remoteproxytestsproxy.h

SOURCES += remoteproxytestsproxy.cpp

target.path = $$[QT_INSTALL_PREFIX]/bin
INSTALLS += target
