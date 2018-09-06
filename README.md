# nymea remote proxy server
----------------------------------------------

The nymea remote proxy server is the meeting point of nymea servers and nymea clients in order to establishing a secure remote connection.

# Build

In order to build the proxy server you need to install the qt default package.

    apt install qt5-default


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


## Build debian package

    $ apt install crossbuilder
    $ cd nymea-remoteproxy
    $ crossbuilder

# Install

## From repository
There is a public version available in the nymea repository.

    $ apt install nymea-remoteproxy nymea-remoteproxy-client nymea-remoteproxy-monitor

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
    authenticationTimeout=8000
    inactiveTimeout=8000
    aloneTimeout=8000
    
    [AWS]
    region=eu-west-1
    authorizerLambdaFunction=system-services-authorizer-dev-checkToken
    awsCredentialsUrl=http://169.254.169.254/latest/meta-data/iam/security-credentials/EC2-Remote-Connection-Proxy-Role
    
    [SSL]
    certificate=/etc/ssl/certs/ssl-cert-snakeoil.pem
    certificateKey=/etc/ssl/private/ssl-cert-snakeoil.key
    certificateChain=
    
    [WebSocketServer]
    host=127.0.0.1
    port=443
    
    [TcpServer]
    host=127.0.0.1
    port=80


# Test

In order to run the test, you can call `make check` in the build directory or run the resulting executable:

    $ nymea-remoteproxy-tests-offline
    $ nymea-remoteproxy-tests-online


## Test coverage report

If you want to create a line coverage report from the tests simply run following command in the source directory:

    $ apt install lcov gcovr
    $ ./create-coverage-html.sh

The resulting coverage report will be place in the `coverage-html` directory.

# Server usage

In order to get information about the server you can start the command with the `--help` parameter.

    $ nymea-remoteproxy --help
    
    Usage: nymea-remoteproxy [options]
    
    The nymea remote proxy server. This server allowes nymea-cloud users and registered nymea deamons to establish a tunnel connection.
    
    Version: 0.1.5
    API version: 0.3
    
    Copyright © 2018 Simon Stürz <simon.stuerz@guh.io>
    
    
    Options:
      -h, --help                           Displays this help.
      -v, --version                        Displays version information.
      -l, --logging <logfile>              Write log file to the given logfile.
      -d, --development                    Enable the development mode. This
                                           enabled the server assumes there are
                                           static AWS credentials provided to
                                           aws-cli.
      -m, --mock-authenticator             Start the server using a mock
                                           authenticator which returns always true.
      -c, --configuration <configuration>  The path to the proxy server
                                           configuration file. The default is
                                           ~/.config/nymea/nymea-remoteproxy.conf
      --verbose                            Print more verbose.
    

# Server API

Once a client connects to the proxy server, he must authenticate him self by passing the token received from the nymea-cloud mqtt connection request.

## Basic flow

#### First client

1. Connect to the proxy server
2. Say hello, in order to know the version, name and API version of the server
3. Authenticate with a token
4. Wait for the tunnel established notification

#### Second client

1. Connect to the proxy server
2. Say hello, in order to know the version, name and API version of the server
3. Authenticate with a token (this has to be the same token as the first client showed)
4. Wait for the tunnel established notification


Once both clients are authenticated, the proxy server will send the `RemoteProxy.TunnelEstablished` notification containing the information of the other tunnel participent. Any traffic coming from he socket is from the remote partner, and any messge sent to the socket will go to the remote partner.

If anything goes wrong, or the tunnel partner disconnects from the proxy, the server will close the other client connection. If any data will be sent between `Authenticate` method and `TunnelEstablished` notification, the server will close the socket.



## Message format

#### Request

    {
        "id": integer,
        "method": "Namespace.Method",
        "o:params" { }
    }

#### Response

    {
        "id": integer,
        "status": "string",
        "o:params" { },
        "o:error": "string"
    }

#### Notification

    {
        "id": integer,
        "notification": "Namespace.Notification",
        "o:params" { }
    }

## Say Hello

#### Request

    {
        "id": 0,
        "method": "RemoteProxy.Hello"
    }


#### Response

    {
        "id": 0,
        "params": {
            "apiVersion": "0.2",
            "name": "community-server",
            "server": "nymea-remoteproxy",
            "version": "0.1.2"
        },
        "status": "success"
    }

## Authenticate the connection

The first data a client **must** send to the proxy server is the authentication request. This request contains the `token` which will be verified agains the nymea-cloud infrastructure and a `nonce` which has to be uniq for each connection attempt and shared between the 2 clients. The `uuid` should be a persistant uuid for this client and the name should make clear which type of connection this is and which client is connecting. The name and uuid will be sent to the tunnel partner during the tunnel establishmend.

#### Request

    {
        "id": 1,
        "method": "Authentication.Authenticate",
        "params": {
            "uuid": "string",
            "name": "string",
            "token": "tokenstring"
            "nonce": "nonce"
        }
    }

#### Response

* **On Success**: If the token was authenticated successfully, the response will look like this:

        {
            "id": 1,
            "status": "success"
        }

* **On Failure** If the token was invalid, the response will look like this and the server will close the connection immediatly:

        {
            "id": 1,
            "status": "error",
            "error": "Invalid token. You are not allowed to use this server."
        }

#### Tunnel established

Once the other client is here and ready, the server will send a notification to the clients indicating that the tunnel has been established successfully. This message is the last data comming from the proxy server.

> **Important:** Any data traffic following after this notification comes from the tunnel endpoint, __not__ from the __proxy server__ any more.

    {
        "id": "0",
        "notification": "RemoteProxy.TunnelEstablished",
        "params": {
            "name": "String",
            "uuid": "String"
        }
    }


## Introspect the API


#### Request

    {
        "id": 0,
        "method": "RemoteProxy.Introspect"
    }

#### Response

    "id": 1,
    "params": {
        "methods": {
            "Authentication.Authenticate": {
                "description": "Authenticate this connection. The returned AuthenticationError informs about the result. If the authentication was not successfull, the server will close the connection immediatly after sending the error response. The given id should be a unique id the other tunnel client can understand. Once the authentication was successfull, you can wait for the RemoteProxy.TunnelEstablished notification. If you send any data before getting this notification, the server will close the connection. If the tunnel client does not show up within 10 seconds, the server will close the connection.",
                "params": {
                    "name": "String",
                    "o:nonce": "String",
                    "token": "String",
                    "uuid": "String"
                },
                "returns": {
                    "authenticationError": "$ref:AuthenticationError"
                }
            },
            "RemoteProxy.Hello": {
                "description": "Once connected to this server, a client can get information about the server by saying Hello. The response informs the client about this proxy server.",
                "params": {
                },
                "returns": {
                    "apiVersion": "String",
                    "name": "String",
                    "server": "String",
                    "version": "String"
                }
            },
            "RemoteProxy.Introspect": {
                "description": "Introspect this API.",
                "params": {
                },
                "returns": {
                    "methods": "Object",
                    "notifications": "Object",
                    "types": "Object"
                }
            }
        },
        "notifications": {
            "RemoteProxy.TunnelEstablished": {
                "description": "Emitted whenever the tunnel has been established successfully. This is the last message from the remote proxy server! Any following data will be from the other tunnel client until the connection will be closed. The parameter contain some information about the other tunnel client.",
                "params": {
                    "name": "String",
                    "uuid": "String"
                }
            }
        },
        "types": {
            "AuthenticationError": [
                "AuthenticationErrorNoError",
                "AuthenticationErrorUnknown",
                "AuthenticationErrorTimeout",
                "AuthenticationErrorAborted",
                "AuthenticationErrorAuthenticationFailed",
                "AuthenticationErrorProxyError"
            ],
            "BasicType": [
                "Uuid",
                "String",
                "Int",
                "UInt",
                "Double",
                "Bool",
                "Variant",
                "Object"
            ]
        }
    },
    "status": "success"
}
    
# Server monitor

The server provides a live monitor interface on a local socket server. You can follow the monitor data with:

    $ sudo socat - UNIX-CONNECT:/tmp/nymea-remoteproxy-monitor.sock

There is also the package `nymea-remoteproxy-monitor` package and application which gives you a nice overview about whats going on on the proxy server.


    $ nymea-remoteproxy-monitor --help
    
    Usage: nymea-remoteproxy-monitor [options]
    
    The nymea remote proxy monitor allowes to monitor the live server activity on the a local instance.
    
    Server version: 0.1.5
    API version: 0.3
    
    Copyright © 2018 Simon Stürz <simon.stuerz@guh.io>
    
    
    Options:
      -h, --help             Displays this help.
      -v, --version          Displays version information.
      -s, --socket <socket>  The socket descriptor for the nymea-remoteproxy
                             monitor socket. Default is
                             /tmp/nymea-remoteproxy-monitor.sock
    
    

# Client usage

The client allowes you to test the proxy server and create a dummy client for testing the connection.


    $ nymea-remoteproxy-client --help
    
    Usage: nymea-remoteproxy-client [options]

    The nymea remote proxy client application. This client allowes to test a server application as client perspective.
    
    Version: 0.1.5
    API version: 0.3
    
    Copyright © 2018 Simon Stürz <simon.stuerz@guh.io>
    
    
    Options:
      -h, --help           Displays this help.
      -v, --version        Displays version information.
      -u, --url <url>      The proxy server url. Default
                           wss://dev-remoteproxy.nymea.io:443
      -t, --token <token>  The AWS token for authentication.
      -i, --igore-ssl      Ignore SSL certificate errors.
      --name <name>        The name of the client. Default nymea-remoteproxyclient
      --uuid <uuid>        The uuid of the client. If not specified, a new one will
                           be created
      --verbose            Print more information about the connection.
      --very-verbose       Print the complete traffic information from the
                           connection.

# Testing a local server

With following steps you can test a local instance with a self signed certificate.

## Start the server

In order to test a connection and play with the server API, you can install the proxy server on your machine and try to connect to it.

    $ sudo apt update
    $ sudo apt install nymea-remoteproxy nymea-remoteproxy-client

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

    $ sudo systemctl stop nymea-remoteproxy.conf

> *Note*: the `-m` starts the proxy with a dummy authenticator, which allowes to use any token, it will always be authenticated and should be used only on localhost running servers.

    $ sudo nymea-remoteproxy -c /etc/nymea/nymea-remoteproxy -m --verbose

## Connect two clients

Once the server is up and running with the dummy authenticator, you can try to connect to the service using the `nymea-remoteproxy-client` in a new terminal.

> *Note:* assuming you are starting the client on the same system as the server:

    $ nymea-remoteproxy-client -i -u wss://localhost -t "blubtoken"

Open a second terminal and start the same command again.

> *Note:* assuming you are starting the client on the same system as the server:

    $ nymea-remoteproxy-client -i -u wss://localhost -t "blubtoken"

You can follow the connection flow on both sides using the `--very-verbose` option.

# License

This is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3 of the License.

