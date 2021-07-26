include(../nymea-remoteproxy.pri)

TARGET = nymea-remoteproxy-monitor
TEMPLATE = app

HEADERS += \
    monitorclient.h \
    terminalwindow.h \
    monitor.h

SOURCES += main.cpp \
    monitorclient.cpp \
    terminalwindow.cpp \
    monitor.cpp

LIBS += -lncurses

target.path = $$[QT_INSTALL_PREFIX]/bin
INSTALLS += target
