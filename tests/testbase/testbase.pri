RESOURCES += ../resources/resources.qrc

include(../../common/common.pri)

CONFIG += testcase
QT += testlib

QMAKE_LFLAGS_RPATH=
QMAKE_LFLAGS += "-Wl,-rpath,\'$${top_srcdir}/libnymea-remoteproxy\':'$${top_srcdir}/libnymea-remoteproxyclient\'"

INCLUDEPATH += $${PWD} $$top_srcdir/libnymea-remoteproxy $$top_srcdir/libnymea-remoteproxyclient
LIBS += -L$$top_builddir/libnymea-remoteproxy/ -lnymea-remoteproxy \
        -L$$top_builddir/libnymea-remoteproxyclient/ -lnymea-remoteproxyclient \

HEADERS += \
    $${PWD}/basetest.h \
    $${PWD}/mockauthenticator.h \

SOURCES += \
    $${PWD}/basetest.cpp \
    $${PWD}/mockauthenticator.cpp \

