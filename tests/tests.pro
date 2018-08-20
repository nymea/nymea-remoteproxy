include(../nymea-remoteproxy.pri)

TEMPLATE=subdirs
SUBDIRS += test-offline

online {
    message("Online tests enabled")
    SUBDIRS += test-online
}

