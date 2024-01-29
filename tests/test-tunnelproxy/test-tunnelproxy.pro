include(../../nymea-remoteproxy.pri)
include(../testbase/testbase.pri)

CONFIG += testcase
QT += testlib

TARGET = tunnelproxy

HEADERS += remoteproxyteststunnelproxy.h

SOURCES += remoteproxyteststunnelproxy.cpp

target.path = $$[QT_INSTALL_PREFIX]/share/tests/nymea-remoteproxy/
INSTALLS += target
