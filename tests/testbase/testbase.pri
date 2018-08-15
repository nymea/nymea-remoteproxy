RESOURCES += ../certificate.qrc

INCLUDEPATH += $${PWD} $$top_srcdir/libnymea-remoteproxy $$top_srcdir/libnymea-remoteproxyclient
LIBS += -L$$top_builddir/libnymea-remoteproxy/ -lnymea-remoteproxy \
        -L$$top_builddir/libnymea-remoteproxyclient/ -lnymea-remoteproxyclient \

HEADERS += \
    $${PWD}/basetest.h \
    $${PWD}/mockauthenticator.h \

SOURCES += \
    $${PWD}/basetest.cpp \
    $${PWD}/mockauthenticator.cpp \

