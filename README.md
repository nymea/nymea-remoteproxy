# nymea remote proxy server
----------------------------------------------

The nymea remote proxy server is the meeting point of nymea servers and nymea clients in order to establishing a secure remote connection.

# Build

In order to build the proxy server you need to install the qt default package.

    apt install qtbase5-dev qtbase5-dev-tools libqt5websockets5-dev libncurses5-dev dpkg-dev debhelper

## Build from source
Change into the source directory and run following commands

    $ cd nymea-remoteproxy
    $ mkdir build
    $ cd build
    $ qmake ../
    $ make -j$(nproc)

In the build directory you can find the resulting library and binary files.

If you want to start the proxy server from the build directory, you need to export the library path before starting the application:

    $ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/libnymea-remoteproxy:$(pwd)/libnymea-remoteproxyclient
    $ ./server/nymea-remoteproxy -c ../nymea-remoteproxy/nymea-remoteproxy.conf


## Embedding the client library in another CMake project

When you need to build the `libnymea-remoteproxyclient` sources directly inside another
project, you can reuse the same source lists that the upstream CMake build relies on.
The repository ships the helper module `cmake/nymea-remoteproxyclient-sources.cmake`
which exposes absolute paths to all `.cpp` and `.h` files as well as the required
include directories. The `libnymea-remoteproxyclient/common` directory is a
relative symlink to the shared `common/` sources so consumers can treat those
files like part of the client library tree.

1. Add this repository as a submodule or vendor drop next to your own sources.
2. From your project, include the helper module and build the library using those lists:

    ```cmake
    # Adjust this path to wherever the nymea-remoteproxy sources live in your tree
    set(NYMEA_REMOTE_PROXY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/nymea-remoteproxy")

    find_package(Qt6 REQUIRED COMPONENTS Core Network WebSockets)

    include("${NYMEA_REMOTE_PROXY_DIR}/cmake/nymea-remoteproxyclient-sources.cmake")

    add_library(nymea-remoteproxyclient STATIC
        ${NYMEA_REMOTEPROXYCLIENT_SOURCES}
    )

    target_link_libraries(nymea-remoteproxyclient
        PUBLIC
            Qt6::Core
            Qt6::Network
            Qt6::WebSockets
    )

    target_include_directories(nymea-remoteproxyclient
        PUBLIC
            ${NYMEA_REMOTEPROXYCLIENT_SOURCE_ROOT}
    )

    configure_file(
        "${NYMEA_REMOTE_PROXY_DIR}/version.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/nymea-remoteproxy/version.h"
        @ONLY
    )

    target_include_directories(nymea-remoteproxyclient
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/nymea-remoteproxy>
    )
    ```

The library target defined above contains all client sources plus the shared
`common/` files. Once you link against `nymea-remoteproxyclient`, the correct
include paths are propagated automatically to consumers of your target.

# Install

## From repository
There is a public version available in the nymea repository.

    $ apt install nymea-remoteproxy nymea-remoteproxy-tunnelclient nymea-remoteproxy-monitor

This will install a systemd service called `nymea-remoteproxy.service` and the client application for testing.

## From build directory

Simply run following command in the build dir:

    $ sudo make install

# Configure

The package will deliver a default configuration file with following content (`/etc/nymea/nymea-remoteproxy.conf`):
    
    [ProxyServer]
    name=nymea-remoteproxy
    writeLogs=false
    logFile=/var/log/nymea-remoteproxy.log
    logEngineEnabled=false
    monitorSocket=/tmp/nymea-remoteproxy-monitor.sock
    jsonRpcTimeout=10000
    inactiveTimeout=8000
    
    [SSL]
    enabled=false
    certificate=/etc/ssl/certs/ssl-cert-snakeoil.pem
    certificateKey=/etc/ssl/private/ssl-cert-snakeoil.key
    certificateChain=
    
    [UnixSocketServerTunnelProxy]
    unixSocketFileName=/run/nymea-remoteproxy.socket
    
    [WebSocketServerTunnelProxy]
    host=127.0.0.1
    port=2212
    
    [TcpServerTunnelProxy]
    host=127.0.0.1
    port=2213
    

## Test coverage report

If you want to create a line coverage report from the tests simply run following command in the source directory:

> Note: run `qmake` with the argument `CONFIG+=converage` in order to generate coverage data for the tests. Then run the tests before creating the coverage report.

    $ apt install lcov gcovr
    $ ./create-coverage-html.sh

The resulting coverage report will be place in the `coverage-html` directory.

# Testing a local server

With following steps you can test a local instance with a self signed certificate.

## Start the server

In order to test a connection and play with the server API, you can install the proxy server on your machine and try to connect to it.

    $ sudo apt update
    $ sudo apt install nymea-remoteproxy nymea-remoteproxy-tunnelclient

Once installed, the `nymea-remoteproxy` will start automatically using the default configurations shipped in `/etc/nymea/nymea-remoteproxy.conf`.
Using this file allowes you to configure the server for your test purposes.

The only thing you need is a certificate, which can be loaded from the server. The server does not support insecure connection for now. If you don't have any certificate, you can create one for testing.

> *Note:* you can enter whatever you like for the certificate.

    $ cd /tmp/
    $ openssl req -x509 -nodes -days 365 -newkey rsa:2048 -keyout test-proxy-certificate.key -out test-proxy-certificate.crt

Now place the certificate and the key where they belong:

    $ sudo cp /tmp/test-proxy-certificate.crt /etc/ssl/certs/
    $ sudo cp /tmp/test-proxy-certificate.key /etc/ssl/private/

Change following configuration in the `/etc/nymea/nymea-remoteproxy.conf`:

    ...
    certificate=/etc/ssl/certs/test-proxy-certificate.crt
    certificateKey=/etc/ssl/private/test-proxy-certificate.key
    ...

Now stop the proxy server and start it manually:

    $ sudo systemctl stop nymea-remoteproxy.service

    $ sudo nymea-remoteproxy -c /etc/nymea/nymea-remoteproxy --verbose


# License

This is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3 of the License.

