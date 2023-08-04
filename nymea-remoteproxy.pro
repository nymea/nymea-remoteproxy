include(nymea-remoteproxy.pri)

# Define versions
SERVER_NAME=nymea-remoteproxy
API_VERSION_MAJOR=0
API_VERSION_MINOR=6
COPYRIGHT_YEAR=2023

# Parse and export SERVER_VERSION
SERVER_VERSION=$$system('dpkg-parsechangelog | sed -n -e "s/^Version: //p"')

QMAKE_SUBSTITUTES += version.h.in

TEMPLATE=subdirs
SUBDIRS += server tunnelclient monitor libnymea-remoteproxy libnymea-remoteproxyclient

!disabletests {
    SUBDIRS += tests
}

server.depends = libnymea-remoteproxy
tunnelclient.depends = libnymea-remoteproxyclient
tests.depends = libnymea-remoteproxy libnymea-remoteproxyclient

test.commands = LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:$$top_builddir/libnymea-remoteproxy:$$top_builddir/libnymea-remoteproxyclient \
                LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:$$top_srcdir/libnymea-remoteproxy:$$top_srcdir/libnymea-remoteproxyclient \
                make check
QMAKE_EXTRA_TARGETS += test

message("----------------------------------------------------------")
message("Building nymea-remoteproxy $${SERVER_VERSION}")
message("----------------------------------------------------------")
message("JSON-RPC API version $${API_VERSION_MAJOR}.$${API_VERSION_MINOR}")
message("Qt version:" $$[QT_VERSION])

coverage {
    message("Building with coverage report")
}

ccache {
    message("Building with ccache support")
}

