include(../../nymea-remoteproxy.pri)
include(../testbase/testbase.pri)

CONFIG += testcase
QT += testlib

TARGET = nymea-remoteproxy-tunnelproxy-tests

HEADERS += remoteproxyteststunnelproxy.h

SOURCES += remoteproxyteststunnelproxy.cpp

target.path = $$[QT_INSTALL_PREFIX]/bin
INSTALLS += target
