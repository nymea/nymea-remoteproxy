include(../nymea-remoteproxy.pri)

TEMPLATE = lib
TARGET = nymea-remoteproxyclient
target.path = /usr/lib/$$system('dpkg-architecture -q DEB_HOST_MULTIARCH')

include(libnymea-remoteproxyclient.pri)

installheaders.files = remoteproxyconnection.h
installheaders.path = /usr/include/nymea-remoteproxyclient/

INSTALLS += target installheaders
