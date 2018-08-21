include(../nymea-remoteproxy.pri)

TARGET = nymea-remoteproxy-monitor
TEMPLATE = app

SOURCES += main.cpp \
    monitorclient.cpp \
    terminalwindow.cpp \
    monitor.cpp

LIBS += -lncurses

target.path = /usr/bin
INSTALLS += target

HEADERS += \
    monitorclient.h \
    terminalwindow.h \
    monitor.h
