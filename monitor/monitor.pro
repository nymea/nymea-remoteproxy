include(../nymea-remoteproxy.pri)

TARGET = nymea-remoteproxy-monitor
TEMPLATE = app

SOURCES += main.cpp

target.path = /usr/bin
INSTALLS += target

HEADERS += \
    proxyclient.h
