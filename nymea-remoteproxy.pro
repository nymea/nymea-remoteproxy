include(nymea-remoteproxy.pri)

TEMPLATE=subdirs
SUBDIRS += server libnymea-remoteproxy libnymea-remoteproxyclient tests

server.depends = libnymea-remoteproxy
tests.depends = libnymea-remoteproxy libnymea-remoteproxyclient

