# nymea remote proxy server

The nymea remote proxy server acts as the rendezvous point between nymea servers and nymea clients and provides a secure tunnel to reach devices that sit behind NAT or firewalled networks.

## Features

- Secure TLS-protected tunnels between nymea instances.
- JSON-RPC based control plane with optional monitoring interface.
- Usable as system service or launched from a build tree for development.

## Requirements

To build the proxy from source, install the Qt development packages and the helper tools that produce the Debian packages:

```
sudo apt install debhelper dpkg-dev pkg-config qt6-base-dev qt6-base-dev-tools \
                 qt6-websockets-dev libncurses5-dev
```

## Build from source

```
cd nymea-remoteproxy
mkdir -p build && cd build
qmake ../
make -j$(nproc)
```

The resulting binaries and libraries will be available in the `build` directory. When running directly from there, the helper libraries must be discoverable:

```
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/libnymea-remoteproxy:$(pwd)/libnymea-remoteproxyclient
./server/nymea-remoteproxy -c ../nymea-remoteproxy/nymea-remoteproxy.conf
```

## Install

### From repository

Official nymea packages are available:

```
sudo apt install nymea-remoteproxy nymea-remoteproxy-tunnelclient nymea-remoteproxy-monitor
```

The installation starts the `nymea-remoteproxy.service` systemd unit and also installs a test client.

### From build directory

```
sudo make install
```

## Configuration

The package ships `/etc/nymea/nymea-remoteproxy.conf` which can be used verbatim or as a template:

```
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
```

## Test coverage

To generate a line coverage report:

- Run `qmake CONFIG+=coverage` and execute the tests to produce the `.gcno`/`.gcda` data.
- Install tooling and generate the HTML report:

```
sudo apt install lcov gcovr
./create-coverage-html.sh
```

The HTML output lives in `coverage-html/`.

## Testing a local server

You can experiment with a local instance by issuing a self-signed certificate and pointing the proxy to it.

```
sudo apt update
sudo apt install nymea-remoteproxy nymea-remoteproxy-tunnelclient
```

After installation the daemon starts automatically using `/etc/nymea/nymea-remoteproxy.conf`, which you can adapt for local testing. Create a throwaway certificate:

```
cd /tmp
openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
        -keyout test-proxy-certificate.key \
        -out test-proxy-certificate.crt
sudo cp test-proxy-certificate.crt /etc/ssl/certs/
sudo cp test-proxy-certificate.key /etc/ssl/private/
```

Adjust the config:

```
certificate=/etc/ssl/certs/test-proxy-certificate.crt
certificateKey=/etc/ssl/private/test-proxy-certificate.key
```

Restart manually for verbose output:

```
sudo systemctl stop nymea-remoteproxy.service
sudo nymea-remoteproxy -c /etc/nymea/nymea-remoteproxy.conf --verbose
```

## License

The nymea-remoteproxy server, monitoring tools, and tunnel client are distributed under the terms of the GNU General Public License version 3. 

The reusable libraries located in `libnymea-remoteproxy` and `libnymea-remoteproxyclient` are provided under the GNU Lesser General Public License version 3 (or later).
See `LICENSE.GPL3` and `LICENSE.LGPL3` for the full texts.
