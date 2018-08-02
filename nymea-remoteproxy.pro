include(nymea-remoteproxy.pri)

TEMPLATE=subdirs
SUBDIRS += server libnymea-remoteproxy tests

server.depends = libnymea-remoteproxy
tests.depends = libnymea-remoteproxy

