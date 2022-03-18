include(../nymea-remoteproxy.pri)

TEMPLATE = lib
TARGET = nymea-remoteproxyclient
target.path = $$[QT_INSTALL_LIBS]

include(libnymea-remoteproxyclient.pri)

installheaders.files = remoteproxyconnection.h
installheaders.path = $$[QT_INSTALL_PREFIX]/include/nymea-remoteproxyclient/

installtunnelheaders.files = tunnelproxy/tunnelproxyremoteconnection.h \
    tunnelproxy/tunnelproxysocket.h \
    tunnelproxy/tunnelproxysocketserver.h
installtunnelheaders.path = $$[QT_INSTALL_PREFIX]/include/nymea-remoteproxyclient/tunnelproxy


INSTALLS += target installheaders installtunnelheaders

# Create pkgconfig file
CONFIG += create_pc create_prl no_install_prl
QMAKE_PKGCONFIG_NAME = libnymea-remoteproxyclient
QMAKE_PKGCONFIG_DESCRIPTION = nymea remoteproxy client development library
QMAKE_PKGCONFIG_PREFIX = $$[QT_INSTALL_PREFIX]
QMAKE_PKGCONFIG_INCDIR = $$[QT_INSTALL_PREFIX]/include/nymea-remoteproxyclient/
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_VERSION = $$SERVER_VERSION
QMAKE_PKGCONFIG_FILE = nymea-remoteproxyclient
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
