include(../nymea-remoteproxy.pri)

TARGET = nymea-remoteproxy
TEMPLATE = app

INCLUDEPATH += ../libnymea-remoteproxy

LIBS += -L$$top_builddir/libnymea-remoteproxy/ -lnymea-remoteproxy

SOURCES += main.cpp \
    remoteproxyserverapplication.cpp

target.path = /usr/bin
INSTALLS += target

HEADERS += \
    remoteproxyserverapplication.h
