include(../../nymea-remoteproxy.pri)
include(../testbase/testbase.pri)

TARGET = nymea-remoteproxy-tests-online

HEADERS += nymea-remoteproxy-tests-online.h

SOURCES += nymea-remoteproxy-tests-online.cpp

target.path = $$[QT_INSTALL_PREFIX]/bin
INSTALLS += target
