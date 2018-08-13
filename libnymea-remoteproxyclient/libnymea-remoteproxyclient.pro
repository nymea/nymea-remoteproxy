include(../nymea-remoteproxy.pri)

TEMPLATE = lib
TARGET = nymea-remoteproxyclient

include(libnymea-remoteproxyclient.pri)

# install header file with relative subdirectory
for(header, HEADERS) {
    path = /usr/include/nymea-remoteproxyclient/$${dirname(header)}
    eval(headers_$${path}.files += $${header})
    eval(headers_$${path}.path = $${path})
    eval(INSTALLS *= headers_$${path})
}
