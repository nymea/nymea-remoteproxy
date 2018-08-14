include(nymea-remoteproxy.pri)

TEMPLATE=subdirs
SUBDIRS += server client libnymea-remoteproxy libnymea-remoteproxyclient tests

server.depends = libnymea-remoteproxy
tests.depends = libnymea-remoteproxy libnymea-remoteproxyclient
client.depends = libnymea-remoteproxyclient

message("----------------------------------------------------------")
message("Building nymea-remoteproxy $${SERVER_VERSION}")
message("----------------------------------------------------------")
message("JSON-RPC API version $${API_VERSION_MAJOR}.$${API_VERSION_MINOR}")
message("Qt version:" $$[QT_VERSION])
coverage { message("Building with coverage report") }
