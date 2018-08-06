#!/bin/bash

# Export the library path for now
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/libnymea-remoteproxy:$(pwd)/libnymea-remoteproxyclient

# Build
qmake CONFIG+=coverage
make -j$(nproc)
make coverage-html

# Clean up source directory
rm -v libnymea-remoteproxy/libnymea-remoteproxy.so*
rm -v libnymea-remoteproxyclient/libnymea-remoteproxyclient.so*
rm -v server/nymea-remoteproxy
rm -v tests/nymea-remoteproxy-tests
make clean
