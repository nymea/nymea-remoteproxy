include(../nymea-remoteproxy.pri)

TEMPLATE=subdirs
SUBDIRS += test-proxy test-tunnelproxy

#online {
#    message("Online tests enabled")
#    SUBDIRS += test-online
#}

