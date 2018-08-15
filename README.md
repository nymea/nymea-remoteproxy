# nymea remote proxy server
----------------------------------------------

The nymea remote proxy server is the meeting point of nymea servers and nymea clients in order to establishing a secure remote connection.

# Build

In order to build the proxy server you need to install the qt default package.

    apt install qt5-default

Change into the source directory and run following commands

    cd nymea-remoteproxy
    mkdir build
    cd build
    qmake ../
    make -j$(nproc)

In the build directory you can find the resulting library and binary files.

If you want to start the proxy server from the build directory, you need to export the library path before starting the application:


    $ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/libnymea-remoteproxy:$(pwd)/libnymea-remoteproxyclient
    $ ./server/nymea-remoteproxy -c ../nymea-remoteproxy/tests/test-certificate.crt -k ../nymea-remoteproxy/tests/test-certificate.key


## AWS SDK

Get the latest source code and build dependecies

    $ apt update
    $ apt install git build-essential cmake libcurl4-openssl-dev libssl-dev uuid-dev zlib1g-dev libpulse-dev
    
    $ git clone https://github.com/aws/aws-sdk-cpp.git

Create the build and install folder

    $ cd aws-sdk-cpp
    $ mkdir -p build/install
    $ cd build

    $ cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_ONLY="lambda" -DCMAKE_INSTALL_PREFIX=$(pwd)/install ../
    $ make -j$(nproc)

Install build output into install directory

    $ make install

#### Building debian package

    $ git clone https://github.com/aws/aws-sdk-cpp.git
    $ cd aws-sdk-cpp

    $ git clone git@gitlab.guh.io:cloud/aws-sdk-cpp-debian.git debian

    $ crossbuilder


# Install



# Configure



# Test

In order to run the test, you can call `make check` in the build directory or run the resulting executable:

    $ nymea-remoteproxy-tests


## Test coverage report

If you want to create a line coverage report from the tests simply run following command in the source directory:


    $ apt install lcov gcovr
    $ ./create-coverage-html.sh

The resulting coverage report will be place in the `coverage-html` directory.

# Usage

In order to get information about the server you can start the command with the `--help` parameter.

    $ nymea-remoteproxy --help

    Usage: nymea-remoteproxy [options]
    
    The nymea remote proxy server. This server allowes nymea-cloud users and registered nymea deamons to establish a tunnel connection.
    
    Server version: 0.0.1
    API version: 0.1
    
    Copyright © 2018 Simon Stürz <simon.stuerz@guh.io>
    
    
    Options:
      -h, --help                               Displays this help.
      -v, --version                            Displays version information.
      -l, --logging <logfile>                  Write log file to the given logfile.
      -s, --server <hostaddress>               The server address this proxy will
                                               listen on. Default is 127.0.0.1
      -p, --port <port>                        The proxy server port. Default is
                                               1212
      -c, --certificate <certificate>          The path to the SSL certificate used
                                               for this proxy server.
      -k, --certificate-key <certificate-key>  The path to the SSL certificate key
                                               used for this proxy server.
      -a, --authentication-server <url>        The server url of the AWS
                                               authentication server.
    

# Server API

Once a client connects to the proxy server, he must authenticate him self by passing the token received from the nymea-cloud mqtt connection request.

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
            "apiVersion": "0.1",
            "name": "nymea-remoteproxy-testserver",
            "server": "nymea-remoteproxy",
            "version": "0.0.1"
        },
        "status": "success"
    }

## Authenticate the connection

The first data a client **must** send to the proxy server is the authentication request. This request contains the token which will be verified agains the nymea-cloud infrastructure.

#### Request

    {
        "id": 1,
        "method": "Authentication.Authenticate",
        "params": {
            "id": "string",
            "name": "string",
            "token": "tokenstring"
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

    
    {
        "id": 0,
        "params": {
            "methods": {
                "Authentication.Authenticate": {
                    "description": "Authenticate this connection. The returned AuthenticationError informs about the result. If the authentication was not successfull, the server will close the connection immediatly after sending the error response. The given id should be a unique id the other tunnel client can understand. Once the authentication was successfull, you can wait for the RemoteProxy.TunnelEstablished notification. If you send any data before getting this notification, the server will close the connection. If the tunnel client does not show up within 10 seconds, the server will close the connection.",
                    "params": {
                        "name": "String",
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
                    "AuthenticationErrorAuthenticationServerNotResponding"
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
    

# License

This is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3 of the License.

