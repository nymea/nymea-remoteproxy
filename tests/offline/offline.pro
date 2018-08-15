include(../../nymea-remoteproxy.pri)
include(../testbase/testbase.pri)

CONFIG += testcase
QT += testlib

TARGET = nymea-remoteproxy-tests-offline

HEADERS += nymea-remoteproxy-tests-offline.h

SOURCES += nymea-remoteproxy-tests-offline.cpp

target.path = /usr/bin
INSTALLS += target
