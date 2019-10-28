include(nymea-remoteproxy.pri)

TEMPLATE=subdirs
SUBDIRS += server client libnymea-remoteproxy libnymea-remoteproxyclient 

!disabletests {
    SUBDIRS+=tests
}

!disablemonitor {
    SUBDIRS+=monitor
}

server.depends = libnymea-remoteproxy
client.depends = libnymea-remoteproxyclient
tests.depends = libnymea-remoteproxy libnymea-remoteproxyclient

message("----------------------------------------------------------")
message("Building nymea-remoteproxy $${SERVER_VERSION}")
message("----------------------------------------------------------")
message("JSON-RPC API version $${API_VERSION_MAJOR}.$${API_VERSION_MINOR}")
message("Qt version:" $$[QT_VERSION])

coverage { message("Building with coverage report") }
ccache { message("Building with ccache support") }
disablemonitor {
    message("Building without the monitor")
}
